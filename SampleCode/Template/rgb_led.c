#include "NuMicro.h"
#include "rgb_led.h"

#define RGB_SPI_FREQUENCY        7000000u

/*
 * One LED protocol bit is expanded to one MSB-first SPI byte.  SPI_Open()
 * selects divider 7 for the 48 MHz PCLK1 when 7 MHz is requested, therefore:
 *
 *   Actual SPI clock = 48 MHz / 7 = 6.857143 MHz
 *   One SPI bit      = 1 / 6.857143 MHz = 145.833 ns
 *
 * 0xC0 = 11000000b gives TH = 2 * 145.833 = 291.667 ns and
 *                         TL = 6 * 145.833 = 875.000 ns.
 * 0xFC = 11111100b gives TH = 6 * 145.833 = 875.000 ns and
 *                         TL = 2 * 145.833 = 291.667 ns.
 * Both encoded bits have a 1.166667 us period.  These values satisfy the
 * LTST-E563CEGBW limits (short 220..380 ns, long 820..980 ns, period
 * 1.04..1.36 us) and the referenced WS2812 limits (short 220..420 ns,
 * long 580..1000 ns).  Keep the SPI source clock, requested frequency and
 * these codes together; changing one requires recalculating all timings.
 */
#define RGB_SPI_CODE_ZERO        0xC0u
#define RGB_SPI_CODE_ONE         0xFCu
#define RGB_PDMA_CHANNEL         0u
/* 256 zero bytes provide 298.667 us low, exceeding LTST 50 us and WS2812 280 us. */
#define RGB_RESET_BYTES          256u
#define RGB_ENCODED_MAX          ((RGB_MAX_LED_COUNT * 24u) + RGB_RESET_BYTES)

static uint8_t s_au8Encoded[RGB_ENCODED_MAX];
static volatile uint8_t s_u8Busy;

static uint8_t RGB_LED_Scale(uint8_t value, uint8_t brightness)
{
    return (uint8_t)(((uint16_t)value * (uint16_t)brightness + 127u) / 255u);
}

void RGB_LED_Init(void)
{
    CLK_EnableModuleClock(SPI0_MODULE);
    CLK_EnableModuleClock(PDMA_MODULE);
    CLK_SetModuleClock(SPI0_MODULE, CLK_CLKSEL2_SPI0SEL_PCLK1, MODULE_NoMsk);
    SYS->GPA_MFPL = (SYS->GPA_MFPL & ~SYS_GPA_MFPL_PA0MFP_Msk) | SYS_GPA_MFPL_PA0MFP_SPI0_MOSI;
    SPI_Open(SPI0, SPI_MASTER, SPI_MODE_0, 8u, RGB_SPI_FREQUENCY);
    PDMA_Open(PDMA, (1u << RGB_PDMA_CHANNEL));
    PDMA_EnableInt(PDMA, RGB_PDMA_CHANNEL, PDMA_INT_TRANS_DONE);
    NVIC_EnableIRQ(PDMA_IRQn);
    s_u8Busy = 0u;
}

uint8_t RGB_LED_IsBusy(void)
{
    return s_u8Busy;
}

uint8_t RGB_LED_Show(uint8_t pixels[RGB_MAX_LED_COUNT][3], uint8_t count,
                     uint8_t order, uint8_t brightness)
{
    uint32_t output;
    uint8_t led;
    uint8_t channel;
    uint8_t bit;
    uint8_t source_channel;
    uint8_t value;
    uint32_t length;

    if ((s_u8Busy != 0u) || (count == 0u) || (count > RGB_MAX_LED_COUNT))
        return 0u;
    output = 0u;
    for (led = 0u; led < count; led++)
    {
        for (channel = 0u; channel < 3u; channel++)
        {
            if (order == RGB_ORDER_GRB)
                source_channel = (channel == 0u) ? 1u : ((channel == 1u) ? 0u : 2u);
            else
                source_channel = channel;
            value = RGB_LED_Scale(pixels[led][source_channel], brightness);
            for (bit = 0u; bit < 8u; bit++)
            {
                s_au8Encoded[output++] = ((value & (uint8_t)(0x80u >> bit)) != 0u) ? RGB_SPI_CODE_ONE : RGB_SPI_CODE_ZERO;
            }
        }
    }
    length = output + RGB_RESET_BYTES;
    while (output < length)
        s_au8Encoded[output++] = 0u;

    PDMA_SetTransferCnt(PDMA, RGB_PDMA_CHANNEL, PDMA_WIDTH_8, length);
    PDMA_SetTransferAddr(PDMA, RGB_PDMA_CHANNEL, (uint32_t)s_au8Encoded, PDMA_SAR_INC,
                         (uint32_t)&SPI0->TX, PDMA_DAR_FIX);
    PDMA_SetTransferMode(PDMA, RGB_PDMA_CHANNEL, PDMA_SPI0_TX, FALSE, 0u);
    PDMA_SetBurstType(PDMA, RGB_PDMA_CHANNEL, PDMA_REQ_SINGLE, 0u);
    s_u8Busy = 1u;
    SPI_TRIGGER_TX_PDMA(SPI0);
    return 1u;
}

void RGB_LED_PdmaIrqHandler(void)
{
    uint32_t status;
    status = PDMA_GET_INT_STATUS(PDMA);
    if ((status & PDMA_INTSTS_TDIF_Msk) != 0u)
    {
        if ((PDMA_GET_TD_STS(PDMA) & (1u << RGB_PDMA_CHANNEL)) != 0u)
        {
            PDMA_CLR_TD_FLAG(PDMA, (1u << RGB_PDMA_CHANNEL));
            SPI_DISABLE_TX_PDMA(SPI0);
            s_u8Busy = 0u;
        }
    }
}
