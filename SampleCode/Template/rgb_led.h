#ifndef RGB_LED_H
#define RGB_LED_H

#include <stdint.h>
#include "rgb_protocol.h"

void RGB_LED_Init(void);
uint8_t RGB_LED_IsBusy(void);
uint8_t RGB_LED_Show(uint8_t pixels[RGB_MAX_LED_COUNT][3], uint8_t count,
                     uint8_t order, uint8_t brightness);
void RGB_LED_PdmaIrqHandler(void);

#endif
