#ifndef RGB_PROTOCOL_H
#define RGB_PROTOCOL_H

#include <stdint.h>

#define RGB_PROTOCOL_MAGIC       0xA5u
#define RGB_PROTOCOL_VERSION     1u
#define RGB_REPORT_SIZE          64u
#define RGB_MAX_LED_COUNT        10u

#define RGB_CMD_GET_INFO         0x01u
#define RGB_CMD_SET_CONFIG       0x02u
#define RGB_CMD_SET_STATIC       0x03u
#define RGB_CMD_SET_PIXELS       0x04u
#define RGB_CMD_SET_EFFECT       0x05u
#define RGB_CMD_STOP             0x06u

#define RGB_ORDER_GRB            0u
#define RGB_ORDER_RGB            1u

#define RGB_EFFECT_STATIC        0u
#define RGB_EFFECT_BREATHING     1u
#define RGB_EFFECT_BLINK         2u
#define RGB_EFFECT_GRADIENT      3u
#define RGB_EFFECT_FLOW          4u

uint8_t RGB_ProtocolCrc8(const uint8_t *data, uint32_t length);
uint8_t RGB_ProtocolValidate(const uint8_t *report);

#endif
