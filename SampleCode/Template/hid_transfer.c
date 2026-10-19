#include <stdio.h>
#include <string.h>
#include "hid_transfer.h"
#include "rgb_protocol.h"
#include "rgb_effect.h"

static uint8_t s_au8Report[RGB_REPORT_SIZE];
static volatile uint8_t s_u8ReportReady;

static const char *HID_EffectName(uint8_t effect)
{
    switch (effect)
    {
    case RGB_EFFECT_STATIC: return "Static";
    case RGB_EFFECT_BREATHING: return "Breathing";
    case RGB_EFFECT_BLINK: return "Blink";
    case RGB_EFFECT_GRADIENT: return "Gradient";
    case RGB_EFFECT_FLOW: return "Flow";
    default: return "Invalid";
    }
}

void USBD_IRQHandler(void)
{
    uint32_t interrupt_status;
    uint32_t bus_state;

    interrupt_status = USBD_GET_INT_FLAG();
    bus_state = USBD_GET_BUS_STATE();
    if ((interrupt_status & USBD_INTSTS_FLDET) != 0u)
    {
        USBD_CLR_INT_FLAG(USBD_INTSTS_FLDET);
        if (USBD_IS_ATTACHED())
            USBD_ENABLE_USB();
        else
            USBD_DISABLE_USB();
    }
    if ((interrupt_status & USBD_INTSTS_BUS) != 0u)
    {
        USBD_CLR_INT_FLAG(USBD_INTSTS_BUS);
        if ((bus_state & USBD_STATE_USBRST) != 0u)
        {
            USBD_ENABLE_USB();
            USBD_SwReset();
        }
        if ((bus_state & USBD_STATE_SUSPEND) != 0u)
            USBD_DISABLE_PHY();
        if ((bus_state & USBD_STATE_RESUME) != 0u)
            USBD_ENABLE_USB();
    }
    if ((interrupt_status & USBD_INTSTS_USB) != 0u)
    {
        if ((interrupt_status & USBD_INTSTS_SETUP) != 0u)
        {
            USBD_CLR_INT_FLAG(USBD_INTSTS_SETUP);
            USBD_STOP_TRANSACTION(EP0);
            USBD_STOP_TRANSACTION(EP1);
            USBD_ProcessSetupPacket();
        }
        if ((interrupt_status & USBD_INTSTS_EP0) != 0u)
        {
            USBD_CLR_INT_FLAG(USBD_INTSTS_EP0);
            USBD_CtrlIn();
        }
        if ((interrupt_status & USBD_INTSTS_EP1) != 0u)
        {
            USBD_CLR_INT_FLAG(USBD_INTSTS_EP1);
            USBD_CtrlOut();
        }
        if ((interrupt_status & USBD_INTSTS_EP2) != 0u)
        {
            USBD_CLR_INT_FLAG(USBD_INTSTS_EP2);
            EP2_Handler();
        }
        if ((interrupt_status & USBD_INTSTS_EP3) != 0u)
        {
            USBD_CLR_INT_FLAG(USBD_INTSTS_EP3);
            EP3_Handler();
        }
    }
}

void EP2_Handler(void)
{
}

void EP3_Handler(void)
{
    uint32_t length;

    length = USBD_GET_PAYLOAD_LEN(EP3);
    if ((length == RGB_REPORT_SIZE) && (s_u8ReportReady == 0u))
    {
        USBD_MemCopy(s_au8Report, (uint8_t *)(USBD_BUF_BASE + USBD_GET_EP_BUF_ADDR(EP3)), RGB_REPORT_SIZE);
        s_u8ReportReady = 1u;
    }
    USBD_SET_PAYLOAD_LEN(EP3, EP3_MAX_PKT_SIZE);
}

void HID_Init(void)
{
    USBD->STBUFSEG = SETUP_BUF_BASE;
    USBD_CONFIG_EP(EP0, USBD_CFG_CSTALL | USBD_CFG_EPMODE_IN | 0u);
    USBD_SET_EP_BUF_ADDR(EP0, EP0_BUF_BASE);
    USBD_CONFIG_EP(EP1, USBD_CFG_CSTALL | USBD_CFG_EPMODE_OUT | 0u);
    USBD_SET_EP_BUF_ADDR(EP1, EP0_BUF_BASE);
    USBD_CONFIG_EP(EP2, USBD_CFG_EPMODE_IN | INT_IN_EP_NUM);
    USBD_SET_EP_BUF_ADDR(EP2, EP2_BUF_BASE);
    USBD_CONFIG_EP(EP3, USBD_CFG_EPMODE_OUT | INT_OUT_EP_NUM);
    USBD_SET_EP_BUF_ADDR(EP3, EP3_BUF_BASE);
    USBD_SET_PAYLOAD_LEN(EP3, EP3_MAX_PKT_SIZE);
}

void HID_ClassRequest(void)
{
    uint8_t setup[8];
    uint8_t request;
    uint8_t request_type;

    USBD_GetSetupPacket(setup);
    request = setup[1];
    request_type = setup[0] & 0x60u;
    if (request_type == 0x20u)
    {
        if (request == 0x01u)
        {
            USBD_PrepareCtrlIn(s_au8Report, RGB_REPORT_SIZE);
            USBD_PrepareCtrlOut(0, 0u);
        }
        else if (request == 0x09u)
        {
            USBD_PrepareCtrlOut(s_au8Report, RGB_REPORT_SIZE);
            USBD_SET_DATA1(EP1);
            USBD_SET_PAYLOAD_LEN(EP1, 0u);
        }
        else
        {
            USBD_SetStall(0u);
        }
    }
}

void HID_Process(void)
{
    uint8_t command;
    uint8_t length;

    if (s_u8ReportReady == 0u)
        return;
    s_u8ReportReady = 0u;
    if (RGB_ProtocolValidate(s_au8Report) == 0u)
    {
        printf("USB RX invalid frame\n");
        return;
    }
    command = s_au8Report[2];
    length = s_au8Report[4];
    if ((command == RGB_CMD_SET_CONFIG) && (length == 3u))
    {
        RGB_EffectSetConfig(s_au8Report[5], s_au8Report[6], s_au8Report[7]);
        printf("USB CMD seq=%u SET_CONFIG count=%u order=%s brightness=%u\n",
               s_au8Report[3], s_au8Report[5], (s_au8Report[6] == RGB_ORDER_RGB) ? "RGB" : "GRB",
               s_au8Report[7]);
    }
    else if ((command == RGB_CMD_SET_STATIC) && (length == 3u))
    {
        RGB_EffectSet(RGB_EFFECT_STATIC, s_au8Report[5], s_au8Report[6], s_au8Report[7], 128u,
                      RGB_EffectGetConfig()->brightness, 0u);
        printf("USB CMD seq=%u SET_STATIC rgb=%u,%u,%u\n", s_au8Report[3], s_au8Report[5],
               s_au8Report[6], s_au8Report[7]);
    }
    else if ((command == RGB_CMD_SET_PIXELS) && (length >= 4u) &&
             (s_au8Report[5] <= RGB_MAX_LED_COUNT) &&
             (length == (uint8_t)(1u + s_au8Report[5] * 3u)))
    {
        RGB_EffectSetPixels(&s_au8Report[6], s_au8Report[5]);
        printf("USB CMD seq=%u SET_PIXELS count=%u\n", s_au8Report[3], s_au8Report[5]);
    }
    else if ((command == RGB_CMD_SET_EFFECT) && (length == 7u))
    {
        RGB_EffectSet(s_au8Report[5], s_au8Report[6], s_au8Report[7], s_au8Report[8],
                      s_au8Report[9], s_au8Report[10], s_au8Report[11]);
        printf("USB CMD seq=%u SET_EFFECT effect=%s(%u) rgb=%u,%u,%u speed=%u brightness=%u direction=%s\n",
               s_au8Report[3], HID_EffectName(s_au8Report[5]), s_au8Report[5], s_au8Report[6],
               s_au8Report[7], s_au8Report[8], s_au8Report[9], s_au8Report[10],
               (s_au8Report[11] != 0u) ? "Reverse" : "Forward");
    }
    else if ((command == RGB_CMD_STOP) && (length == 0u))
    {
        RGB_EffectStop();
        printf("USB CMD seq=%u STOP\n", s_au8Report[3]);
    }
    else if ((command == RGB_CMD_GET_INFO) && (length == 0u))
    {
        printf("USB CMD seq=%u GET_INFO reserved-no-response\n", s_au8Report[3]);
    }
    else
    {
        printf("USB CMD seq=%u unsupported command=0x%02X length=%u\n", s_au8Report[3], command, length);
    }
}
