/**
 * @file gpio.h
 * @brief GPIO driver API for the STM32G4
 *
 * Provides alternate-function selection, direction control, and read/write
 * operations for the STM32G4 GPIO ports, backed by the ST HAL.
 *
 * No portable contract: STM32 pins carry a port, an alternate function, an
 * output type, and a pull, which do not fit the RP2040's function-select
 * signature. Pins are packed with BM_GPIO_PIN from gpio_platform.h.
 *
 * Reference: RM0440 §9 (GPIO), ST HAL stm32g4xx_hal_gpio
 */

#ifndef GPIO_H
#define GPIO_H



#include <stddef.h>
#include <stdint.h>
#include <stdbool.h>
#include "gpio_platform.h"


// returns false if error, true if functioned correctly

// enable the clock for the pin's GPIO port
// called by every configuration function below, so callers rarely need it directly
bool bm_gpio_enable(uint8_t pin);

// route the pin to alternate function af (0-15)
// output_type: BM_GPIO_PUSH_PULL or BM_GPIO_OPEN_DRAIN
// pull:        BM_GPIO_PULL_NONE, BM_GPIO_PULL_UP, or BM_GPIO_PULL_DOWN
bool bm_gpio_set_function(uint8_t pin, uint8_t af, uint8_t output_type, uint8_t pull);

// configure the pin as a push-pull output or floating input
// direction: GPIO_DIR_INPUT or GPIO_DIR_OUTPUT
bool bm_gpio_set_direction(uint8_t pin, uint8_t direction);

// Set output value high or low
bool bm_gpio_put(uint8_t pin, bool value);

// Read input value into value
bool bm_gpio_get(uint8_t pin, bool *value);

//force pin high
bool bm_gpio_set(uint8_t pin);

//forces a pin low
bool bm_gpio_clear(uint8_t pin);

//flips pin
bool bm_gpio_toggle(uint8_t pin);


#endif /* GPIO_H */
