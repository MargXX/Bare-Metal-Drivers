#include <stdint.h>
#include <stdbool.h>

// Set the function of a pin to SIO (software-controlled GPIO)
// You need this before using a pin as GPIO rather than I2C/SPI/UART
void gpio_set_function(uint8_t pin, uint8_t function);

// Set pin direction: 1 = output, 0 = input
void gpio_set_direction(uint8_t pin, uint8_t direction);

// Set output value high or low
void gpio_put(uint8_t pin, bool value);

// Read input value (returns 0 or 1)
bool gpio_get(uint8_t pin);

// Optional but useful: set/clear using atomic SET/CLR registers
// (avoids read-modify-write race conditions)
void gpio_set(uint8_t pin);
void gpio_clear(uint8_t pin);
void gpio_toggle(uint8_t pin);

