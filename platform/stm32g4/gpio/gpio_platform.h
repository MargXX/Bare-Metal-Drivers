/**
 * @file gpio_platform.h
 * @brief Public platform constants for STM32G4 GPIO
 *
 * STM32 pins are addressed by port and pin number. Drivers on this target
 * take a single uint8_t pin, packed as (port << 4) | pin, so the UART and I2C
 * init signatures keep the same shape as on the RP2040. Build one with
 * BM_GPIO_PIN(BM_GPIO_PORT_A, 5) for PA5.
 *
 * Names are prefixed BM_ to stay clear of the HAL's GPIO_* macros.
 */


#ifndef GPIO_PLATFORM_H
#define GPIO_PLATFORM_H

// GPIO ports present on the STM32G431xx (RM0440 memory map, GPIOA-GPIOG)
#define BM_GPIO_PORT_A  0
#define BM_GPIO_PORT_B  1
#define BM_GPIO_PORT_C  2
#define BM_GPIO_PORT_D  3
#define BM_GPIO_PORT_E  4
#define BM_GPIO_PORT_F  5
#define BM_GPIO_PORT_G  6

// pack port and pin into the uint8_t pin argument used by every STM32 driver
#define BM_GPIO_PIN(port, num)  ((uint8_t)((((port) & 0x0F) << 4) | ((num) & 0x0F)))
#define BM_GPIO_PIN_PORT(pin)   (((pin) >> 4) & 0x0F)
#define BM_GPIO_PIN_NUM(pin)    ((pin) & 0x0F)

// GPIO Direction
#define GPIO_DIR_INPUT  0
#define GPIO_DIR_OUTPUT 1

// Output type for alternate-function pins
#define BM_GPIO_PUSH_PULL   0
#define BM_GPIO_OPEN_DRAIN  1

// Pull resistor selection for alternate-function pins
#define BM_GPIO_PULL_NONE   0
#define BM_GPIO_PULL_UP     1
#define BM_GPIO_PULL_DOWN   2

// Port, pin, and alternate function limits
#define MAX_PORT_NUMBER     BM_GPIO_PORT_G
#define MAX_PIN_NUMBER      15
#define MAX_FUNCTION_NUMBER 15  // AF0-AF15

#endif /* GPIO_PLATFORM_H */
