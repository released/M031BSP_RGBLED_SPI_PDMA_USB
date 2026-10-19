/******************************************************************************
 * @file     main.c
 * @brief    M032TD2AE USB HID RGB LED controller using SPI0 and PDMA.
 ******************************************************************************/
#include <stdio.h>
#include "NuMicro.h"
#include "hid_transfer.h"
#include "rgb_led.h"
#include "rgb_effect.h"

static void SYS_Init(void)
{
    CLK_EnableXtalRC(CLK_PWRCTL_HIRCEN_Msk);
    CLK_WaitClockReady(CLK_STATUS_HIRCSTB_Msk);
    CLK_SetHCLK(CLK_CLKSEL0_HCLKSEL_HIRC, CLK_CLKDIV0_HCLK(1));

    CLK_EnableModuleClock(UART0_MODULE);
    CLK_EnableModuleClock(TMR0_MODULE);
    CLK_EnableModuleClock(USBD_MODULE);
    CLK_SetModuleClock(UART0_MODULE, CLK_CLKSEL1_UART0SEL_HIRC, CLK_CLKDIV0_UART0(1));
    CLK_SetModuleClock(TMR0_MODULE, CLK_CLKSEL1_TMR0SEL_PCLK0, MODULE_NoMsk);
    CLK_SetModuleClock(USBD_MODULE, CLK_CLKSEL0_USBDSEL_HIRC, CLK_CLKDIV0_USB(1));

    /* PB.12 = UART0_RXD and PB.13 = UART0_TXD for debug. */
    SYS->GPB_MFPH = (SYS->GPB_MFPH & ~(SYS_GPB_MFPH_PB12MFP_Msk | SYS_GPB_MFPH_PB13MFP_Msk)) |
                    SYS_GPB_MFPH_PB12MFP_UART0_RXD | SYS_GPB_MFPH_PB13MFP_UART0_TXD;
    SystemCoreClockUpdate();
}

void TMR0_IRQHandler(void)
{
    TIMER_ClearIntFlag(TIMER0);
    RGB_EffectTick1ms();
}

void PDMA_IRQHandler(void)
{
    RGB_LED_PdmaIrqHandler();
}

int32_t main(void)
{
    SYS_UnlockReg();
    SYS_Init();
    UART_Open(UART0, 115200u);
    RGB_LED_Init();
    RGB_EffectInit();

    TIMER_Open(TIMER0, TIMER_PERIODIC_MODE, 1000u);
    TIMER_EnableInt(TIMER0);
    NVIC_EnableIRQ(TMR0_IRQn);
    TIMER_Start(TIMER0);

    USBD_Open(&gsInfo, HID_ClassRequest, 0);
    HID_Init();
    USBD_Start();
    NVIC_EnableIRQ(USBD_IRQn);
    SYS_LockReg();

    printf("M032 RGB controller v1.0, HID 0416:0125, PA.0 SPI0 MOSI\n");
    while (1)
    {
        HID_Process();
        RGB_EffectProcess();
    }
}
