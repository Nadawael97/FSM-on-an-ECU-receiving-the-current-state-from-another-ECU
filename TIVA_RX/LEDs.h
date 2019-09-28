#ifndef __RGBLED_H__
#define __RGBLED_H__

#include <stdbool.h>
#include <stdint.h>
#include "inc/hw_memmap.h"
#include "inc/hw_types.h"
#include "inc/hw_gpio.h"
#include "driverlib/sysctl.h"
#include "driverlib/gpio.h"
#include "driverlib/rom.h"
#include "buttons.h"
#include "utils/uartstdio.h"

#define RED_GPIO_PIN            GPIO_PIN_1
#define BLUE_GPIO_PIN           GPIO_PIN_2
#define GREEN_GPIO_PIN          GPIO_PIN_3
#define WHITE_GPIO_PIN GPIO_PIN_1 | GPIO_PIN_2 | GPIO_PIN_3

extern void LED_Init(void);
extern void LED_ON(uint32_t, uint32_t);

#endif