#include "gpio.h"

#define MAX_PIN_NUMBER 29 // pico exposes 30 pins, numbered 0-29
#define MAX_FUNCTION_NUMBER 9 //functions 0-9

#define GPIO_FUNC_SPIO 1
#define GPIO_FUNC_UART 2
#define GPIO_FUNC_I2C 3
#define GPIO_FUNC_PWM 4
#define GPIO_FUNC_SIO 5
#define GPIO_FUNC_PIO0 6
#define GPIO_FUNC_PIO1 7
#define GPIO_FUNC_CLOCK 8
#define GPIO_FUNC_USB 9
#define GPIO_FUNC_NULL 31

#define GPIO_DIR_INPUT  0
#define GPIO_DIR_OUTPUT 1

#define GPIO_IN 1

#define GPIO_OUT 4
#define GPIO_OUT_SET 5
#define GPIO_OUT_CLR 6
#define GPIO_OUT_XOR 7

#define GPIO_OE  8
#define GPIO_OE_SET  9
#define GPIO_OE_CLR  10

//GPIO pin registers come in pairs: status/ctrl
typedef struct {  
    uint32_t status;
    uint32_t ctrl;
} gpio_pin_regs_t;
//8 bytes per pin struct

//register pointer for the whole bank of GPIO pins, which we can index into with pin number to get to the right registers
// volitile tell compiler not to optimize access to these registers
volatile gpio_pin_regs_t *io_bank0 = (volatile gpio_pin_regs_t *)0x40014000;
volatile uint32_t *sio = (volatile uint32_t *)0xd0000000;

bool gpio_set_function(uint8_t pin, uint8_t function) {
    // Implementation to set the function of the GPIO pin
    // returns true if successful, false if invalid pin or function

    if (pin > MAX_PIN_NUMBER) {
        return false; // Invalid pin number
    }
    if (function > MAX_FUNCTION_NUMBER && function != GPIO_FUNC_NULL) {
        return false; // Invalid function
    }

    //clear bits 4:0 
    uint32_t mask = 0b00011111; 
    io_bank0[pin].ctrl &= ~mask; //keep all bits but mask
    function &= mask; //keep only bits of mask
    io_bank0[pin].ctrl |= function; 
    return true;
}

// Set pin direction: 1 = output, 0 = input
bool gpio_set_direction(uint8_t pin, uint8_t direction) {

    if (pin > MAX_PIN_NUMBER) {
        return false; // Invalid pin number
    }
    if (direction != GPIO_DIR_INPUT && direction != GPIO_DIR_OUTPUT) {
        return false; // Invalid direction
    }

    //setting logic
    uint32_t mask = 1u << pin; 
    if (direction == GPIO_DIR_OUTPUT) {
        sio[GPIO_OE_SET] = mask;
    } else {
        sio[GPIO_OE_CLR] = mask;
    }

    return true;
}

// Set output value high or low, true for high, false for low
bool gpio_put(uint8_t pin, bool value) {

    if (pin > MAX_PIN_NUMBER) {return false;} // Invalid pin number

    //setting logic
    uint32_t mask = 1u << pin; 
    if (value) {
        sio[GPIO_OUT_SET] = mask;
    } else {
        sio[GPIO_OUT_CLR] = mask;
    }

    return true;
}

// Read input value (returns 0 or 1)
bool gpio_get(uint8_t pin, bool *value) {
    if (pin > MAX_PIN_NUMBER) {return false;} // Invalid pin number

    uint32_t mask = 1u << pin; 
    *value =  (sio[GPIO_IN] & mask) != 0;

    return true;
}

//enables GPIO on input with SIO
bool gpio_enable(uint8_t pin) {

    if (pin > MAX_PIN_NUMBER) {return false;} // Invalid pin number

    return gpio_set_function(pin, GPIO_FUNC_SIO) && 
    gpio_set_direction(pin, GPIO_DIR_INPUT);
}

//force pin high
bool gpio_set(uint8_t pin) {

    if (pin > MAX_PIN_NUMBER) {return false;} // Invalid pin number

    //setting logic
    uint32_t mask = 1u << pin; 
    sio[GPIO_OUT_SET] = mask;

    return true;
}

//force pin low
bool gpio_clear(uint8_t pin) {

    if (pin > MAX_PIN_NUMBER) {return false;} // Invalid pin number

    //setting logic
    uint32_t mask = 1u << pin; 
    sio[GPIO_OUT_CLR] = mask;

    return true;
}

//flips pin
bool gpio_toggle(uint8_t pin) {

    if (pin > MAX_PIN_NUMBER) {return false;} // Invalid pin number

    //setting logic
    uint32_t mask = 1u << pin; 
    sio[GPIO_OUT_XOR] = mask;

    return true;
}