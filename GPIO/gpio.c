#include "gpio.h"
#include "gpio_reg.h"



bool bm_gpio_set_function(uint8_t pin, uint8_t function) {
    // Implementation to set the function of the GPIO pin
    // returns true if successful, false if invalid pin or function

    if (pin > MAX_PIN_NUMBER) {
        return false; // Invalid pin number
    }
    if (function > MAX_FUNCTION_NUMBER && function != GPIO_FUNC_NULL) {
        return false; // Invalid function
    }

    //clear bits 4:0 
    uint32_t mask = IO_BANK_Msk; 
    IO_BANK0[pin].ctrl &= ~mask; //keep all bits but mask
    function &= mask; //keep only bits of mask
    IO_BANK0[pin].ctrl |= function; 
    return true;
}

// Set pin direction: 1 = output, 0 = input
bool bm_gpio_set_direction(uint8_t pin, uint8_t direction) {

    if (pin > MAX_PIN_NUMBER) {
        return false; // Invalid pin number
    }
    if (direction != GPIO_DIR_INPUT && direction != GPIO_DIR_OUTPUT) {
        return false; // Invalid direction
    }

    //setting logic
    uint32_t mask = 1u << pin; 
    if (direction == GPIO_DIR_OUTPUT) {
        SIO[GPIO_OE_SET] = mask;
    } else {
        SIO[GPIO_OE_CLR] = mask;
    }

    return true;
}

// Set output value high or low, true for high, false for low
bool bm_gpio_put(uint8_t pin, bool value) {

    if (pin > MAX_PIN_NUMBER) {return false;} // Invalid pin number

    //setting logic
    uint32_t mask = 1u << pin; 
    if (value) {
        SIO[GPIO_OUT_SET] = mask;
    } else {
        SIO[GPIO_OUT_CLR] = mask;
    }

    return true;
}

// Read input value (returns 0 or 1)
bool bm_gpio_get(uint8_t pin, bool *value) {
    if (pin > MAX_PIN_NUMBER) {return false;} // Invalid pin number
    if (value == NULL) {return false;}

    uint32_t mask = 1u << pin; 
    *value =  (SIO[GPIO_IN] & mask) != 0;

    return true;
}

//enables GPIO on input with SIO
bool bm_gpio_enable(uint8_t pin) {

    if (pin > MAX_PIN_NUMBER) {return false;} // Invalid pin number

    return bm_gpio_set_function(pin, GPIO_FUNC_SIO) && 
    bm_gpio_set_direction(pin, GPIO_DIR_INPUT);
}

//force pin high
bool bm_gpio_set(uint8_t pin) {

    if (pin > MAX_PIN_NUMBER) {return false;} // Invalid pin number

    //setting logic
    uint32_t mask = 1u << pin; 
    SIO[GPIO_OUT_SET] = mask;

    return true;
}

//force pin low
bool bm_gpio_clear(uint8_t pin) {

    if (pin > MAX_PIN_NUMBER) {return false;} // Invalid pin number

    //setting logic
    uint32_t mask = 1u << pin; 
    SIO[GPIO_OUT_CLR] = mask;

    return true;
}

//flips pin
bool bm_gpio_toggle(uint8_t pin) {

    if (pin > MAX_PIN_NUMBER) {return false;} // Invalid pin number

    //setting logic
    uint32_t mask = 1u << pin; 
    SIO[GPIO_OUT_XOR] = mask;

    return true;
}