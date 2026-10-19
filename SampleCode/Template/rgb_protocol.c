#include "rgb_protocol.h"

uint8_t RGB_ProtocolCrc8(const uint8_t *data, uint32_t length)
{
    uint32_t i;
    uint8_t bit;
    uint8_t crc;

    crc = 0u;
    for (i = 0u; i < length; i++)
    {
        crc ^= data[i];
        for (bit = 0u; bit < 8u; bit++)
        {
            if ((crc & 0x80u) != 0u)
                crc = (uint8_t)((crc << 1) ^ 0x07u);
            else
                crc <<= 1;
        }
    }
    return crc;
}

uint8_t RGB_ProtocolValidate(const uint8_t *report)
{
    if ((report[0] != RGB_PROTOCOL_MAGIC) || (report[1] != RGB_PROTOCOL_VERSION))
        return 0u;
    if (report[4] > (RGB_REPORT_SIZE - 6u))
        return 0u;
    return (RGB_ProtocolCrc8(report, RGB_REPORT_SIZE - 1u) == report[RGB_REPORT_SIZE - 1u]) ? 1u : 0u;
}
