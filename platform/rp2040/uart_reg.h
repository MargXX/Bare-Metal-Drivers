/**
 * @file uart_reg.h
 * @brief UART register map for the RP2040
 * includes pin pair definitions for peripheral selection and register struct definitions for direct access
 *
 * Reference: RP2040 Datasheet, Sections 4.2
 */

#ifndef UART_REG_H
#define UART_REG_H

#include <stdint.h>


// Valid UART pin pairs (RP2040 IO mux table, Section 2.19)
// UART peripheral is determined by pin assignment — not configured independently
//   UART0: (0,1), (12,13), (16,17), (28,29)
//   UART1: (4,5), (8,9),  (20,21), (24,25)

typedef struct {  
    uint8_t tx;
    uint8_t rx;
    uint8_t peripheral;
} uart_pin_pair;

//this is here so uart.c is device portable
static const uart_pin_pair uart_valid_pairs[] = {
    { .tx =  0, .rx =  1, .peripheral = 0 },
    { .tx = 12, .rx = 13, .peripheral = 0 },
    { .tx = 16, .rx = 17, .peripheral = 0 },
    { .tx = 28, .rx = 29, .peripheral = 0 },
    { .tx =  4, .rx =  5, .peripheral = 1 },
    { .tx =  8, .rx =  9, .peripheral = 1 },
    { .tx = 20, .rx = 21, .peripheral = 1 },
    { .tx = 24, .rx = 25, .peripheral = 1 },
}; 

#define NUM_UART_PAIRS (sizeof(uart_valid_pairs) / sizeof(uart_pin_pair))

#define UART0_BASE 0x40034000
#define UART1_BASE 0x40038000

//register offsets from base
typedef struct {
    volatile uint32_t UARTDR;           // 0x00 Data Register
    volatile uint32_t UARTRSR;          // 0x04 Receive Status Register
    volatile uint32_t _reserved0[4];    // 0x08-0x17 reserved
    volatile uint32_t UARTFR;           // 0x18 Flag Register
    volatile uint32_t _reserved1[1];    // 0x1C-0x1F reserved
    volatile uint32_t UARTILPR;         // 0x20 IrDA Low-Power Counter
    volatile uint32_t UARTIBRD;         // 0x24 Integer Baud Rate Divisor
    volatile uint32_t UARTFBRD;         // 0x28 Fractional Baud Rate Divisor
    volatile uint32_t UARTLCR;          // 0x2C Line Control Register
    volatile uint32_t UARTCR;           // 0x30 Control Register
    volatile uint32_t UARTIFLS;         // 0x34 Interrupt FIFO Level Select
    volatile uint32_t UARTIMSC;         // 0x38 Interrupt Mask Set/Clear
    volatile uint32_t UARTRIS;          // 0x3C Raw Interrupt Status
    volatile uint32_t UARTMIS;          // 0x40 Masked Interrupt Status
    volatile uint32_t UARTICR;          // 0x44 Interrupt Clear Register
    volatile uint32_t UARTDMACR;        // 0x48 DMA Control Register
    volatile uint32_t _reserved2[997];  // 0x4C-0xFDF reserved
    volatile uint32_t UARTPeriphID[4];  // 0xFE0-0xFEC Peripheral ID Registers
    volatile uint32_t UARTPCellID[4];   // 0xFF0-0xFFC PrimeCell ID Registers
} uart_regs_t;

// Register pointer for the first UART peripheral (UART0) — use to access registers by name, e.g. UART->UARTDR, index for UART1
#define UART_NUM_PERIPHERALS 2

static volatile uart_regs_t * const uart_peripherals[] = {
    (volatile uart_regs_t *)UART0_BASE,
    (volatile uart_regs_t *)UART1_BASE,
};


// FUARTCLK ≤ 5/3 × FPCLK
#define UARTCLK 125000000UL // 125MHz clock default

#define BAUD_RATE_MIN (UARTCLK / 16 / 65535) // min baud rate is when IBRD=65535 and FBRD=63
#define BAUD_RATE_MAX (UARTCLK / 16) // max baud rate is when IBRD=1 and FBRD=0

// Register masks
#define UARTIBRD_Msk ((1UL << 16) - 1) // 16 bits for integer baud rate divisor
#define UARTFBRD_Msk ((1UL << 6) - 1)  // 6 bits for fractional baud rate divisor

#define UARTCR_UARTEN_Msk   (1UL << 0) // UART Enable
#define UARTCR_TXE_Msk  (1UL << 8) // Transmit Enable
#define UARTCR_RXE_Msk  (1UL << 9) // Receive Enable





#endif /* UART_REG_H */