#include <string.h>
#include "rgb_effect.h"
#include "rgb_led.h"

static RGB_EFFECT_CONFIG_T s_sConfig;
static uint8_t s_au8Pixels[RGB_MAX_LED_COUNT][3];
static volatile uint8_t s_u8FrameDue;
static volatile uint16_t s_u16Elapsed;
static uint16_t s_u16Phase;
static uint8_t s_u8DirectPixels;
static uint8_t s_u8ClearCount;

static void RGB_EffectFill(uint8_t red, uint8_t green, uint8_t blue)
{
    uint8_t index;
    for (index = 0u; index < s_sConfig.led_count; index++)
    {
        s_au8Pixels[index][0] = red;
        s_au8Pixels[index][1] = green;
        s_au8Pixels[index][2] = blue;
    }
}

static uint8_t RGB_EffectScale(uint8_t value, uint8_t level)
{
    return (uint8_t)(((uint16_t)value * level) / 255u);
}

static void RGB_EffectWheel(uint8_t position, uint8_t *red, uint8_t *green, uint8_t *blue)
{
    if (position < 85u)
    {
        *red = (uint8_t)(255u - position * 3u); *green = (uint8_t)(position * 3u); *blue = 0u;
    }
    else if (position < 170u)
    {
        position = (uint8_t)(position - 85u);
        *red = 0u; *green = (uint8_t)(255u - position * 3u); *blue = (uint8_t)(position * 3u);
    }
    else
    {
        position = (uint8_t)(position - 170u);
        *red = (uint8_t)(position * 3u); *green = 0u; *blue = (uint8_t)(255u - position * 3u);
    }
}

void RGB_EffectInit(void)
{
    memset(&s_sConfig, 0, sizeof(s_sConfig));
    s_sConfig.led_count = 8u;
    s_sConfig.order = RGB_ORDER_GRB;
    s_sConfig.brightness = 128u;
    s_sConfig.effect = RGB_EFFECT_STATIC;
    s_sConfig.red = 32u;
    s_u8DirectPixels = 0u;
    s_u8ClearCount = 0u;
    s_u8FrameDue = 1u;
}

void RGB_EffectSetConfig(uint8_t count, uint8_t order, uint8_t brightness)
{
    uint8_t old_count;
    uint8_t index;

    old_count = s_sConfig.led_count;
    if ((count >= 1u) && (count <= RGB_MAX_LED_COUNT))
    {
        if ((count < old_count) && (old_count > s_u8ClearCount))
            s_u8ClearCount = old_count;
        s_sConfig.led_count = count;
        if ((s_u8DirectPixels != 0u) && (count > old_count))
        {
            for (index = old_count; index < count; index++)
            {
                s_au8Pixels[index][0] = 0u;
                s_au8Pixels[index][1] = 0u;
                s_au8Pixels[index][2] = 0u;
            }
        }
    }
    if (order <= RGB_ORDER_RGB) s_sConfig.order = order;
    s_sConfig.brightness = brightness;
    /* Keep the active effect/direct-pixel source when only strip configuration changes. */
    s_u8FrameDue = 1u;
}

void RGB_EffectSet(uint8_t effect, uint8_t red, uint8_t green, uint8_t blue,
                   uint8_t speed, uint8_t brightness, uint8_t direction)
{
    if (effect > RGB_EFFECT_FLOW) effect = RGB_EFFECT_STATIC;
    s_sConfig.effect = effect; s_sConfig.red = red; s_sConfig.green = green; s_sConfig.blue = blue;
    s_sConfig.speed = (speed == 0u) ? 1u : speed; s_sConfig.brightness = brightness;
    s_sConfig.direction = (direction != 0u) ? 1u : 0u;
    s_u8DirectPixels = 0u;
    s_u16Phase = 0u; s_u16Elapsed = 0u; s_u8FrameDue = 1u;
}

void RGB_EffectSetPixels(const uint8_t *rgb, uint8_t count)
{
    if ((count == 0u) || (count > RGB_MAX_LED_COUNT)) return;
    s_sConfig.led_count = count; s_sConfig.effect = RGB_EFFECT_STATIC;
    memcpy(s_au8Pixels, rgb, (uint32_t)count * 3u);
    s_u8DirectPixels = 1u;
    s_u8FrameDue = 1u;
}

void RGB_EffectStop(void)
{
    s_sConfig.effect = RGB_EFFECT_STATIC; s_sConfig.red = 0u; s_sConfig.green = 0u; s_sConfig.blue = 0u;
    s_u8DirectPixels = 0u;
    RGB_EffectFill(0u, 0u, 0u); s_u8FrameDue = 1u;
}

void RGB_EffectTick1ms(void)
{
    uint16_t interval;
    if (s_sConfig.effect == RGB_EFFECT_BLINK)
        interval = (uint16_t)(50u + (uint16_t)(255u - s_sConfig.speed) * 4u);
    else
        interval = (uint16_t)(4u + (uint16_t)(255u - s_sConfig.speed) / 4u);
    s_u16Elapsed++;
    if (s_u16Elapsed >= interval) { s_u16Elapsed = 0u; s_u16Phase++; s_u8FrameDue = 1u; }
}

void RGB_EffectProcess(void)
{
    uint8_t index;
    uint8_t level;
    uint8_t position;
    uint8_t red;
    uint8_t green;
    uint8_t blue;
    uint8_t output_count;

    if ((s_u8FrameDue == 0u) || (RGB_LED_IsBusy() != 0u)) return;
    s_u8FrameDue = 0u;
    if ((s_sConfig.effect == RGB_EFFECT_STATIC) && (s_u8DirectPixels == 0u))
        RGB_EffectFill(s_sConfig.red, s_sConfig.green, s_sConfig.blue);
    else if (s_sConfig.effect == RGB_EFFECT_BREATHING)
    {
        position = (uint8_t)(s_u16Phase & 0xFFu);
        level = (position < 128u) ? (uint8_t)(position * 2u) : (uint8_t)((255u - position) * 2u);
        RGB_EffectFill(RGB_EffectScale(s_sConfig.red, level), RGB_EffectScale(s_sConfig.green, level), RGB_EffectScale(s_sConfig.blue, level));
    }
    else if (s_sConfig.effect == RGB_EFFECT_BLINK)
    {
        if ((s_u16Phase & 1u) != 0u) RGB_EffectFill(s_sConfig.red, s_sConfig.green, s_sConfig.blue);
        else RGB_EffectFill(0u, 0u, 0u);
    }
    else
    {
        for (index = 0u; index < s_sConfig.led_count; index++)
        {
            position = (uint8_t)(s_u16Phase + ((uint16_t)index * 256u / s_sConfig.led_count));
            if (s_sConfig.direction != 0u) position = (uint8_t)(255u - position);
            if (s_sConfig.effect == RGB_EFFECT_GRADIENT)
            {
                level = (position < 128u) ? (uint8_t)(position * 2u) :
                                            (uint8_t)((255u - position) * 2u);
                s_au8Pixels[index][0] = RGB_EffectScale(s_sConfig.red, level);
                s_au8Pixels[index][1] = RGB_EffectScale(s_sConfig.green, level);
                s_au8Pixels[index][2] = RGB_EffectScale(s_sConfig.blue, level);
            }
            else
            {
                RGB_EffectWheel(position, &red, &green, &blue);
                s_au8Pixels[index][0] = red; s_au8Pixels[index][1] = green; s_au8Pixels[index][2] = blue;
            }
        }
    }
    output_count = s_sConfig.led_count;
    if (s_u8ClearCount > output_count)
    {
        for (index = output_count; index < s_u8ClearCount; index++)
        {
            s_au8Pixels[index][0] = 0u;
            s_au8Pixels[index][1] = 0u;
            s_au8Pixels[index][2] = 0u;
        }
        output_count = s_u8ClearCount;
    }
    if (RGB_LED_Show(s_au8Pixels, output_count, s_sConfig.order, s_sConfig.brightness) != 0u)
        s_u8ClearCount = 0u;
}

const RGB_EFFECT_CONFIG_T *RGB_EffectGetConfig(void) { return &s_sConfig; }
