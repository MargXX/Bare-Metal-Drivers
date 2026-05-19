/**
 * @file gpio_platform.h
 * @brief Public platform constants for RP2040 GPIO
 */


#ifndef GPIO_PLATFORM_H
#define GPIO_PLATFORM_H

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

// Pin and Function Limits
#define MAX_PIN_NUMBER      29  // Pico exposes 30 pins, numbered 0-29
#define MAX_FUNCTION_NUMBER 9   // functions 0-9

#endif