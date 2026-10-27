#include <string.h>
#include "uart.h"
#include "uart_reg.h"
#include "uart_platform.h"
#include "gpio.h"
#include "hal_glue.h"
#include "stm32g4xx_hal.h"

// returns false on error, true on success

// one HAL handle per entry in uart_instances; Instance stays NULL until init
static UART_HandleTypeDef uart_handles[NUM_UART_PERIPHERALS];

// true if uart_num names a peripheral that bm_uart_init has brought up
static bool uart_is_ready(uint8_t uart_num) {
    if (uart_num >= NUM_UART_PERIPHERALS) { return false; }
    return uart_handles[uart_num].gState != HAL_UART_STATE_RESET;
}

// gate the peripheral clock — single consumer, so kept inline with the driver
static void uart_clock_enable(uint8_t peripheral) {
    switch (peripheral) {
        case 0: __HAL_RCC_USART1_CLK_ENABLE(); break;
        case 1: __HAL_RCC_USART2_CLK_ENABLE(); break;
        case 2: __HAL_RCC_USART3_CLK_ENABLE(); break;
        case 3: __HAL_RCC_UART4_CLK_ENABLE(); break;
        case 4: __HAL_RCC_LPUART1_CLK_ENABLE(); break;
        default: break;
    }
}

// initialize UART peripheral as 8N1 — must be called before any other UART functions
// tx_pin and rx_pin must be a valid UART-capable pin pair for the target peripheral
// returns false if pin combination is invalid, the timebase is not running,
// or the HAL rejects the baud rate for this peripheral's clock
bool bm_uart_init(uint8_t *uart_num, uint32_t baud_rate, uint8_t tx_pin, uint8_t rx_pin) {
    if (uart_num == NULL) { return false; }
    if (baud_rate == 0) { return false; } //invalid baud rate
    // every HAL timeout below reads the SysTick millisecond count
    if (!bm_hal_timebase_ready()) { return false; }

    // validate pin pair and determine peripheral
    const uart_pin_pair *pair = NULL;
    for (size_t i = 0; i < NUM_UART_PAIRS; i++) {
        if (uart_valid_pairs[i].tx == tx_pin && uart_valid_pairs[i].rx == rx_pin) {
            pair = &uart_valid_pairs[i];
            break;
        }
    }
    if (pair == NULL) { return false; } //invalid pin pair

    // peripheral clock source is left at its reset value (RCC_CCIPR = PCLK)
    uart_clock_enable(pair->peripheral);

    // configure GPIO pins for UART function, pull-up on RX so an unconnected
    // line idles high rather than reading as a stream of break characters
    if (!bm_gpio_set_function(tx_pin, pair->af, BM_GPIO_PUSH_PULL, BM_GPIO_PULL_NONE)) { return false; }
    if (!bm_gpio_set_function(rx_pin, pair->af, BM_GPIO_PUSH_PULL, BM_GPIO_PULL_UP)) { return false; }

    UART_HandleTypeDef *h = &uart_handles[pair->peripheral];
    memset(h, 0, sizeof(*h));
    h->Instance = uart_instances[pair->peripheral];
    h->Init.BaudRate = baud_rate;
    h->Init.WordLength = UART_WORDLENGTH_8B;
    h->Init.StopBits = UART_STOPBITS_1;
    h->Init.Parity = UART_PARITY_NONE;
    h->Init.Mode = UART_MODE_TX_RX;
    h->Init.HwFlowCtl = UART_HWCONTROL_NONE;
    h->Init.OverSampling = UART_OVERSAMPLING_16;
    h->Init.OneBitSampling = UART_ONE_BIT_SAMPLE_DISABLE;
    h->Init.ClockPrescaler = UART_PRESCALER_DIV1;
    h->AdvancedInit.AdvFeatureInit = UART_ADVFEATURE_NO_INIT;

    // HAL computes BRR from the live PCLK and returns HAL_ERROR if it is out of range
    if (HAL_UART_Init(h) != HAL_OK) {
        // HAL leaves gState BUSY on a config error; clear it so uart_is_ready
        // does not treat a failed init as a usable peripheral
        memset(h, 0, sizeof(*h));
        return false;
    }

    *uart_num = pair->peripheral; //output the peripheral number for use in other functions
    return true; //success
}

// transmit a single byte — blocks until the byte has been sent
bool bm_uart_write_byte(uint8_t uart_num, uint8_t byte) {
    if (!uart_is_ready(uart_num)) { return false; }
    return HAL_UART_Transmit(&uart_handles[uart_num], &byte, 1, UART_TX_TIMEOUT_MS) == HAL_OK;
}

// transmit len bytes from buf — blocks until all bytes are written
bool bm_uart_write(uint8_t uart_num, const uint8_t *buf, size_t len) {
    if (buf == NULL) { return false; }
    for (size_t i = 0; i < len; i++) {
        if (!bm_uart_write_byte(uart_num, buf[i])) {return false;}
    }
    return true; //success
}

// transmit a null-terminated string — blocks until all bytes are written
bool bm_uart_write_str(uint8_t uart_num, const char *str) {
    if (str == NULL) { return false; }
    for (size_t i = 0; str[i] != '\0'; i++) {
        if (!bm_uart_write_byte(uart_num, (uint8_t)str[i])) {return false;}
    }
    return true; //success
}

// receive a single byte into byte_out — blocks until data arrives or UART_RX_TIMEOUT_MS
bool bm_uart_read_byte(uint8_t uart_num, uint8_t *byte_out) {
    if (byte_out == NULL) { return false; }
    if (!uart_is_ready(uart_num)) { return false; }
    // a stale overrun from bytes nobody read would otherwise be reported
    // as an error on this read
    __HAL_UART_CLEAR_OREFLAG(&uart_handles[uart_num]);
    return HAL_UART_Receive(&uart_handles[uart_num], byte_out, 1, UART_RX_TIMEOUT_MS) == HAL_OK;
}

// receive up to len bytes into buf — blocks until len bytes are received
bool bm_uart_read(uint8_t uart_num, uint8_t *buf, size_t len) {
    if (buf == NULL) { return false; }
    for (size_t i = 0; i < len; i++) {
        if (!bm_uart_read_byte(uart_num, &buf[i])) {return false;}
    }
    return true; //success
}

// returns true if the transmit data register is full — use to check before writing if non-blocking behavior is needed
// FIFO mode is left disabled, so this is the single-entry TDR (USART_ISR.TXE clear)
// returns true for an uninitialized uart_num, since nothing can be written to it
bool bm_uart_tx_full(uint8_t uart_num) {
    if (!uart_is_ready(uart_num)) { return true; }
    return __HAL_UART_GET_FLAG(&uart_handles[uart_num], UART_FLAG_TXE) == RESET;
}

// returns true if the receive data register has data available — use to poll before reading
bool bm_uart_rx_ready(uint8_t uart_num) {
    if (!uart_is_ready(uart_num)) { return false; }
    return __HAL_UART_GET_FLAG(&uart_handles[uart_num], UART_FLAG_RXNE) != RESET;
}

// reads USART_ISR and the HAL error code into status_out and error_out
// useful for diagnosing framing errors, overrun, noise, or parity errors
bool bm_uart_get_status(uint8_t uart_num, uint32_t *status_out, uint32_t *error_out) {
    if (status_out == NULL) { return false; }
    if (error_out == NULL) { return false; }
    if (!uart_is_ready(uart_num)) { return false; }
    *status_out = uart_handles[uart_num].Instance->ISR;
    *error_out = HAL_UART_GetError(&uart_handles[uart_num]);
    return true; // success
}
