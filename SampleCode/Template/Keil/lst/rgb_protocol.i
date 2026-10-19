# 1 "../rgb_protocol.c"
# 1 "<built-in>" 1
# 1 "<built-in>" 3
# 382 "<built-in>" 3
# 1 "<command line>" 1
# 1 "<built-in>" 2
# 1 "../rgb_protocol.c" 2
# 1 "..\\rgb_protocol.h" 1



# 1 "C:\\Keil_v5\\ARM\\ARMCLANG\\bin\\..\\include\\stdint.h" 1 3
# 56 "C:\\Keil_v5\\ARM\\ARMCLANG\\bin\\..\\include\\stdint.h" 3
typedef signed char int8_t;
typedef signed short int int16_t;
typedef signed int int32_t;
typedef signed long long int int64_t;


typedef unsigned char uint8_t;
typedef unsigned short int uint16_t;
typedef unsigned int uint32_t;
typedef unsigned long long int uint64_t;





typedef signed char int_least8_t;
typedef signed short int int_least16_t;
typedef signed int int_least32_t;
typedef signed long long int int_least64_t;


typedef unsigned char uint_least8_t;
typedef unsigned short int uint_least16_t;
typedef unsigned int uint_least32_t;
typedef unsigned long long int uint_least64_t;




typedef signed int int_fast8_t;
typedef signed int int_fast16_t;
typedef signed int int_fast32_t;
typedef signed long long int int_fast64_t;


typedef unsigned int uint_fast8_t;
typedef unsigned int uint_fast16_t;
typedef unsigned int uint_fast32_t;
typedef unsigned long long int uint_fast64_t;






typedef signed int intptr_t;
typedef unsigned int uintptr_t;



typedef signed long long intmax_t;
typedef unsigned long long uintmax_t;
# 5 "..\\rgb_protocol.h" 2
# 27 "..\\rgb_protocol.h"
uint8_t RGB_ProtocolCrc8(const uint8_t *data, uint32_t length);
uint8_t RGB_ProtocolValidate(const uint8_t *report);
# 2 "../rgb_protocol.c" 2

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
    if ((report[0] != 0xA5u) || (report[1] != 1u))
        return 0u;
    if (report[4] > (64u - 6u))
        return 0u;
    return (RGB_ProtocolCrc8(report, 64u - 1u) == report[64u - 1u]) ? 1u : 0u;
}
