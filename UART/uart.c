#include "uart.h"
#include "uart_reg.h"

// returns false on error, true on success

// initialize UART peripheral — must be called before any other UART functions
// tx_pin and rx_pin must be a valid UART-capable GPIO pair for the target peripheral
// returns false if pin combination is invalid or peripheral is already enabled
bool bm_uart_init(uint8_t *uart_num, uint32_t baud_rate, uint8_t tx_pin, uint8_t rx_pin) {
    //validate baud rate
    if (baud_rate < BAUD_RATE_MIN || baud_rate > BAUD_RATE_MAX) {
        return false; //invalid baud rate
    }
    // validate pin pair and determine peripheral
    uint8_t peripheral = 255; //invalid peripheral value
    for (size_t i = 0; i < NUM_UART_PAIRS; i++) {
        if (uart_valid_pairs[i].tx == tx_pin && uart_valid_pairs[i].rx == rx_pin) {
            peripheral = uart_valid_pairs[i].peripheral;
            *uart_num = peripheral; //output the peripheral number for use in other functions
            break;
        }
    }
    if (peripheral == 255) { return false; } //invalid pin pair
    
    //disable UART to allow configuration
    uart_peripherals[*uart_num]->UARTCR &= ~UARTCR_UARTEN_Msk; 
    // calculate and set baud rate divisors in UARTIBRD and UARTFBRD
    uint32_t ibrd = UARTCLK / (16 * baud_rate); 
    uint32_t fbrd = ((UARTCLK % (16 * baud_rate)) * 64 + 8 * baud_rate) / (16 * baud_rate);
    // write reserved bits as 0 and mask to field size to ensure no overflow into reserved bits
    uart_peripherals[*uart_num]->UARTIBRD = ibrd & UARTIBRD_Msk;
    uart_peripherals[*uart_num]->UARTFBRD = fbrd & UARTFBRD_Msk;
    // configure GPIO pins for UART function
    bm_gpio_set_function(tx_pin, GPIO_FUNC_UART);
    bm_gpio_set_function(rx_pin, GPIO_FUNC_UART);
    // enable FIFOs, set word length to 8 bits, parity and stop bits to defaults (no parity, 1 stop bit)
    // enable UART, TX, and RX

    return false; //not implemented yet
}

// transmit a single byte — blocks until TX FIFO has space
bool bm_uart_write_byte(uint8_t uart_num,uint8_t byte) {
    return false; //not implemented yet
}

// transmit len bytes from buf — blocks until all bytes are written
bool bm_uart_write(uint8_t uart_num, const uint8_t *buf, size_t len) {
    return false; //not implemented yet
}

// transmit a null-terminated string — blocks until all bytes are written
bool bm_uart_write_str(uint8_t uart_num, const char *str) {
    return false; //not implemented yet
}

// receive a single byte into byte_out — blocks until RX FIFO has data
bool bm_uart_read_byte(uint8_t uart_num, uint8_t *byte_out) {
    return false; //not implemented yet
}

// receive up to len bytes into buf — blocks until len bytes are received
bool bm_uart_read(uint8_t uart_num, uint8_t *buf, size_t len) {
    return false; //not implemented yet
}

// returns true if the TX FIFO is full — use to check before writing if non-blocking behavior is needed
bool bm_uart_tx_full(uint8_t uart_num) {
    return false; //not implemented yet
}

// returns true if the RX FIFO has data available — use to poll before reading
bool bm_uart_rx_ready(uint8_t uart_num) {
    return false; //not implemented yet
}

// reads UARTRIS (raw interrupt status) and UARTFR (flag register) into status_out and flags_out
// useful for diagnosing framing errors, overrun, break conditions, or FIFO state
bool bm_uart_get_status(uint8_t uart_num, uint32_t *status_out, uint32_t *flags_out) {
    return false; //not implemented yet
}