#ifndef HID_TRANSFER_H
#define HID_TRANSFER_H

#include <stdint.h>
#include "NuMicro.h"

#define USBD_VID                 0x0416u
#define USBD_PID                 0x0125u
#define EP0_MAX_PKT_SIZE         64u
#define EP2_MAX_PKT_SIZE         64u
#define EP3_MAX_PKT_SIZE         64u
#define INT_IN_EP_NUM            1u
#define INT_OUT_EP_NUM           2u

#define SETUP_BUF_BASE           0u
#define SETUP_BUF_LEN            8u
#define EP0_BUF_BASE             (SETUP_BUF_BASE + SETUP_BUF_LEN)
#define EP0_BUF_LEN              EP0_MAX_PKT_SIZE
#define EP2_BUF_BASE             (EP0_BUF_BASE + EP0_BUF_LEN)
#define EP3_BUF_BASE             (EP2_BUF_BASE + EP2_MAX_PKT_SIZE)

extern const S_USBD_INFO_T gsInfo;

void HID_Init(void);
void HID_ClassRequest(void);
void EP2_Handler(void);
void EP3_Handler(void);
void HID_Process(void);

#endif
