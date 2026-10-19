#ifndef RGB_EFFECT_H
#define RGB_EFFECT_H

#include <stdint.h>
#include "rgb_protocol.h"

typedef struct
{
    uint8_t led_count;
    uint8_t order;
    uint8_t brightness;
    uint8_t effect;
    uint8_t red;
    uint8_t green;
    uint8_t blue;
    uint8_t speed;
    uint8_t direction;
} RGB_EFFECT_CONFIG_T;

void RGB_EffectInit(void);
void RGB_EffectSetConfig(uint8_t count, uint8_t order, uint8_t brightness);
void RGB_EffectSet(uint8_t effect, uint8_t red, uint8_t green, uint8_t blue,
                   uint8_t speed, uint8_t brightness, uint8_t direction);
void RGB_EffectSetPixels(const uint8_t *rgb, uint8_t count);
void RGB_EffectStop(void);
void RGB_EffectTick1ms(void);
void RGB_EffectProcess(void);
const RGB_EFFECT_CONFIG_T *RGB_EffectGetConfig(void);

#endif
