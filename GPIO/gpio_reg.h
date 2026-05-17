/**
 * @file gpio_reg.h
 * @brief GPIO register map, pin constants, and function definitions for the RP2040
 *
 * Reference: RP2040 Datasheet, Sections 2.3 (SIO), 2.19 (GPIO)
 */

#ifndef GPIO_REG_H
#define GPIO_REG_H

#include <stdint.h>

// Pin and Function Limits
#define MAX_PIN_NUMBER      29  // Pico exposes 30 pins, numbered 0-29
#define MAX_FUNCTION_NUMBER 9   // functions 0-9

// GPIO Function Select
#define GPIO_FUNC_SPI0  1
#define GPIO_FUNC_UART  2
#define GPIO_FUNC_I2C   3
#define GPIO_FUNC_PWM   4
#define GPIO_FUNC_SIO   5
#define GPIO_FUNC_PIO0  6
#define GPIO_FUNC_PIO1  7
#define GPIO_FUNC_CLOCK 8
#define GPIO_FUNC_USB   9
#define GPIO_FUNC_NULL  31

// GPIO Direction
#define GPIO_DIR_INPUT  0
#define GPIO_DIR_OUTPUT 1

// SIO GPIO Register Offsets
#define GPIO_IN      1

#define GPIO_OUT     4
#define GPIO_OUT_SET 5
#define GPIO_OUT_CLR 6
#define GPIO_OUT_XOR 7

#define GPIO_OE      8
#define GPIO_OE_SET  9
#define GPIO_OE_CLR  10

#endif /* GPIO_REG_H */