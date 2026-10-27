#include "gpio.h"
#include "gpio_platform.h"
#include "stm32g4xx_hal.h"

// returns false on error, true on success unless said otherwise in function comment

// indexed by BM_GPIO_PORT_x
static GPIO_TypeDef *const gpio_ports[] = {
    GPIOA, GPIOB, GPIOC, GPIOD, GPIOE, GPIOF, GPIOG,
};

#define NUM_GPIO_PORTS (sizeof(gpio_ports) / sizeof(gpio_ports[0]))

// returns the port for a packed pin, or NULL if the port does not exist
// pin number needs no check: BM_GPIO_PIN_NUM masks it to 0-15
static GPIO_TypeDef *gpio_port_of(uint8_t pin) {
    uint8_t port = BM_GPIO_PIN_PORT(pin);
    if (port >= NUM_GPIO_PORTS) { return NULL; }
    return gpio_ports[port];
}

static uint16_t gpio_mask_of(uint8_t pin) {
    return (uint16_t)(1U << BM_GPIO_PIN_NUM(pin));
}

// enable the clock for the pin's GPIO port
bool bm_gpio_enable(uint8_t pin) {
    switch (BM_GPIO_PIN_PORT(pin)) {
        case BM_GPIO_PORT_A: __HAL_RCC_GPIOA_CLK_ENABLE(); break;
        case BM_GPIO_PORT_B: __HAL_RCC_GPIOB_CLK_ENABLE(); break;
        case BM_GPIO_PORT_C: __HAL_RCC_GPIOC_CLK_ENABLE(); break;
        case BM_GPIO_PORT_D: __HAL_RCC_GPIOD_CLK_ENABLE(); break;
        case BM_GPIO_PORT_E: __HAL_RCC_GPIOE_CLK_ENABLE(); break;
        case BM_GPIO_PORT_F: __HAL_RCC_GPIOF_CLK_ENABLE(); break;
        case BM_GPIO_PORT_G: __HAL_RCC_GPIOG_CLK_ENABLE(); break;
        default: return false; //invalid port
    }
    return true;
}

// route the pin to alternate function af (0-15)
bool bm_gpio_set_function(uint8_t pin, uint8_t af, uint8_t output_type, uint8_t pull) {
    GPIO_TypeDef *port = gpio_port_of(pin);
    if (port == NULL) { return false; } //invalid port
    if (af > MAX_FUNCTION_NUMBER) { return false; }

    GPIO_InitTypeDef init = {0};
    init.Pin = gpio_mask_of(pin);
    init.Alternate = af;
    // UART and I2C both run well under the low-speed slew limit
    init.Speed = GPIO_SPEED_FREQ_LOW;

    if (output_type == BM_GPIO_PUSH_PULL) {
        init.Mode = GPIO_MODE_AF_PP;
    } else if (output_type == BM_GPIO_OPEN_DRAIN) {
        init.Mode = GPIO_MODE_AF_OD;
    } else {
        return false; //invalid output type
    }

    if (pull == BM_GPIO_PULL_NONE) {
        init.Pull = GPIO_NOPULL;
    } else if (pull == BM_GPIO_PULL_UP) {
        init.Pull = GPIO_PULLUP;
    } else if (pull == BM_GPIO_PULL_DOWN) {
        init.Pull = GPIO_PULLDOWN;
    } else {
        return false; //invalid pull
    }

    if (!bm_gpio_enable(pin)) { return false; }
    HAL_GPIO_Init(port, &init);
    return true;
}

// configure the pin as a push-pull output or floating input
bool bm_gpio_set_direction(uint8_t pin, uint8_t direction) {
    GPIO_TypeDef *port = gpio_port_of(pin);
    if (port == NULL) { return false; } //invalid port

    GPIO_InitTypeDef init = {0};
    init.Pin = gpio_mask_of(pin);
    init.Pull = GPIO_NOPULL;
    init.Speed = GPIO_SPEED_FREQ_LOW;

    if (direction == GPIO_DIR_OUTPUT) {
        init.Mode = GPIO_MODE_OUTPUT_PP;
    } else if (direction == GPIO_DIR_INPUT) {
        init.Mode = GPIO_MODE_INPUT;
    } else {
        return false; //invalid direction
    }

    if (!bm_gpio_enable(pin)) { return false; }
    HAL_GPIO_Init(port, &init);
    return true;
}

// Set output value high or low
bool bm_gpio_put(uint8_t pin, bool value) {
    GPIO_TypeDef *port = gpio_port_of(pin);
    if (port == NULL) { return false; }
    HAL_GPIO_WritePin(port, gpio_mask_of(pin), value ? GPIO_PIN_SET : GPIO_PIN_RESET);
    return true;
}

// Read input value into value
bool bm_gpio_get(uint8_t pin, bool *value) {
    if (value == NULL) { return false; }
    GPIO_TypeDef *port = gpio_port_of(pin);
    if (port == NULL) { return false; }
    *value = (HAL_GPIO_ReadPin(port, gpio_mask_of(pin)) == GPIO_PIN_SET);
    return true;
}

//force pin high
bool bm_gpio_set(uint8_t pin) {
    return bm_gpio_put(pin, true);
}

//forces a pin low
bool bm_gpio_clear(uint8_t pin) {
    return bm_gpio_put(pin, false);
}

//flips pin
bool bm_gpio_toggle(uint8_t pin) {
    GPIO_TypeDef *port = gpio_port_of(pin);
    if (port == NULL) { return false; }
    HAL_GPIO_TogglePin(port, gpio_mask_of(pin));
    return true;
}
