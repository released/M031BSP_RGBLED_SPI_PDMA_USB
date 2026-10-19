#include "NuMicro.h"
#include "hid_transfer.h"

#define LEN_CONFIG_AND_SUBORDINATE (LEN_CONFIG + LEN_INTERFACE + LEN_HID + (LEN_ENDPOINT * 2u))

static uint8_t s_au8HidReportDescriptor[] = {
    0x06, 0x00, 0xFF, 0x09, 0x01, 0xA1, 0x01,
    0x15, 0x00, 0x26, 0xFF, 0x00, 0x75, 0x08,
    0x95, 0x40, 0x09, 0x01, 0x81, 0x02,
    0x95, 0x40, 0x09, 0x01, 0x91, 0x02, 0xC0
};

static uint8_t s_au8DeviceDescriptor[] = {
    LEN_DEVICE, DESC_DEVICE, 0x10, 0x01, 0x00, 0x00, 0x00, EP0_MAX_PKT_SIZE,
    (uint8_t)(USBD_VID & 0xFFu), (uint8_t)(USBD_VID >> 8),
    (uint8_t)(USBD_PID & 0xFFu), (uint8_t)(USBD_PID >> 8),
    0x00, 0x01, 0x01, 0x02, 0x03, 0x01
};

static uint8_t s_au8ConfigDescriptor[] = {
    LEN_CONFIG, DESC_CONFIG, (uint8_t)LEN_CONFIG_AND_SUBORDINATE, 0x00, 0x01, 0x01, 0x00, 0x80, 50,
    LEN_INTERFACE, DESC_INTERFACE, 0x00, 0x00, 0x02, 0x03, 0x00, 0x00, 0x00,
    LEN_HID, DESC_HID, 0x11, 0x01, 0x00, 0x01, DESC_HID_RPT,
    (uint8_t)sizeof(s_au8HidReportDescriptor), 0x00,
    LEN_ENDPOINT, DESC_ENDPOINT, (INT_IN_EP_NUM | EP_INPUT), EP_INT, EP2_MAX_PKT_SIZE, 0x00, 0x01,
    LEN_ENDPOINT, DESC_ENDPOINT, (INT_OUT_EP_NUM | EP_OUTPUT), EP_INT, EP3_MAX_PKT_SIZE, 0x00, 0x01
};

static uint8_t s_au8Language[] = {4, DESC_STRING, 0x09, 0x04};
static uint8_t s_au8Vendor[] = {16, DESC_STRING, 'N',0,'u',0,'v',0,'o',0,'t',0,'o',0,'n',0};
static uint8_t s_au8Product[] = {40, DESC_STRING, 'M',0,'0',0,'3',0,'2',0,' ',0,'R',0,'G',0,'B',0,' ',0,'C',0,'o',0,'n',0,'t',0,'r',0,'o',0,'l',0,'l',0,'e',0,'r',0};
static uint8_t s_au8Serial[] = {16, DESC_STRING, 'M',0,'0',0,'3',0,'2',0,'R',0,'G',0,'B',0};
static uint8_t *s_apu8Strings[] = {s_au8Language, s_au8Vendor, s_au8Product, s_au8Serial};
static uint8_t *s_apu8Reports[] = {s_au8HidReportDescriptor, 0};
static uint32_t s_au32ReportLengths[] = {sizeof(s_au8HidReportDescriptor), 0u};
static uint32_t s_au32HidIndexes[] = {LEN_CONFIG + LEN_INTERFACE, 0u};

const S_USBD_INFO_T gsInfo = {
    s_au8DeviceDescriptor, s_au8ConfigDescriptor, s_apu8Strings, s_apu8Reports, 0,
    s_au32ReportLengths, s_au32HidIndexes
};
