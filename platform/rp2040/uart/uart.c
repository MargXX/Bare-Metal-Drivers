#include "uart.h"
#include "uart_reg.h"
#include "resets_reg.h"
#include "gpio.h"

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
    

    //dissasert reset  - RP2040 SPECIFIC
    RESETS->RESET &= ~uart_resets_reset_mask[*uart_num];
    // poll until reset is done
    uint32_t timeout = 0x0FFFFFFF;
    while (((RESETS->DONE & uart_resets_reset_done_mask[*uart_num])) == 0) {
        if (timeout == 0) {return false;}
        timeout--;
    }

    //disable UART to allow configuration
    uart_peripherals[*uart_num]->UARTCR &= ~(UARTCR_UARTEN_Msk | UARTCR_SIREN_Msk | UARTCR_SIRLP_Msk); //also disable SIR low power mode just in case, as recommended by datasheet
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
    uart_peripherals[*uart_num]->UARTLCR_H = UARTLCR_H_FEN_Msk | UARTLCR_H_WLEN_8BIT;
    // enable UART, TX, and RX
    uart_peripherals[*uart_num]->UARTCR |= UARTCR_UARTEN_Msk | UARTCR_TXE_Msk | UARTCR_RXE_Msk;

    return true; //success
}

// transmit a single byte — blocks until TX FIFO has space
bool bm_uart_write_byte(uint8_t uart_num, uint8_t byte) {
    
    // poll to see if TX is ready
    uint32_t timeout = 0x0FFFFFFF;
    while (bm_uart_tx_full(uart_num)) {
        if (timeout == 0) {return false;}
        timeout--;
    }

    uart_peripherals[uart_num]->UARTDR = byte;
    
    return true; //success
}

// transmit len bytes from buf — blocks until all bytes are written
bool bm_uart_write(uint8_t uart_num, const uint8_t *buf, size_t len) {
    for (uint16_t i = 0; i < len; i++) {
        if (!bm_uart_write_byte(uart_num, buf[i])) {return false;}
    }
    return true; //success
}

// transmit a null-terminated string — blocks until all bytes are written
bool bm_uart_write_str(uint8_t uart_num, const char *str) {

    for (uint16_t i = 0; str[i] != '\0'; i++) {
        if (!bm_uart_write_byte(uart_num, str[i])) {return false;}
    }
    return true; //success
}

// receive a single byte into byte_out — blocks until RX FIFO has data
bool bm_uart_read_byte(uint8_t uart_num, uint8_t *byte_out) {
    // poll to see if RX is ready
    uint32_t timeout = 0x0FFFFFFF;
    while (!bm_uart_rx_ready(uart_num)) {
        if (timeout == 0) {return false;}
        timeout--;
    }

    *byte_out = uart_peripherals[uart_num]->UARTDR & UARTDR_DATA_Msk;
    // reading also clears error flags in UARTRSR
    
    return true; //success
}

// receive up to len bytes into buf — blocks until len bytes are received
bool bm_uart_read(uint8_t uart_num, uint8_t *buf, size_t len) {
    for (uint16_t i = 0; i < len; i++) {
        if (!bm_uart_read_byte(uart_num, &buf[i])) {return false;}
    }
    return true; //success
}

// returns true if the TX FIFO is full — use to check before writing if non-blocking behavior is needed
bool bm_uart_tx_full(uint8_t uart_num) {
    
    return (((uart_peripherals[uart_num]->UARTFR & UARTFR_TXFF_MsK)) != 0);
}

// returns true if the RX FIFO has data available — use to poll before reading
bool bm_uart_rx_ready(uint8_t uart_num) {
    return ((uart_peripherals[uart_num]->UARTFR & UARTFR_RXFE_MsK)) == 0;
}

// reads UARTRIS (raw interrupt status) and UARTFR (flag register) into status_out and flags_out
// useful for diagnosing framing errors, overrun, break conditions, or FIFO state
bool bm_uart_get_status(uint8_t uart_num, uint32_t *status_out, uint32_t *flags_out) {
    *status_out = uart_peripherals[uart_num]->UARTRIS;
    *flags_out = uart_peripherals[uart_num]->UARTFR;
    return true; // success
}