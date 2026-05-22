/**
 * @file resets_reg.h
 * @brief Reset register map for the RP2040
 * includes pin pair definitions for peripheral selection and register struct definitions for direct access
 *
 * Reference: RP2040 Datasheet, Sections 2.14
 */

#ifndef RESETS_REG_H
#define RESETS_REG_H

#include <stdint.h>

#define RESETS_BASE 0x4000c000 

typedef struct {
    volatile uint32_t RESET;    //RESETS_RESETS_OFFSET 0x0
    volatile uint32_t WDSEL;    //RESETS_WDSEL_OFFSET 0x4
    volatile uint32_t DONE;     //RESETS_RESET_DONE_OFFSET 0x8
} reset_regs_t;

#define RESETS ((volatile reset_regs_t *)RESETS_BASE)

// Register masks
#define RESETS_ADC_Msk              (1UL << 0)
#define RESETS_BUSCTRL_Msk          (1UL << 1)
#define RESETS_DMA_Msk              (1UL << 2)
#define RESETS_I2C0_Msk             (1UL << 3)
#define RESETS_I2C1_Msk             (1UL << 4)
#define RESETS_IOBANK_Msk           (1UL << 5)
#define RESETS_IO_QSPI_BANK0_Msk    (1UL << 6)
#define RESETS_JTAG_Msk             (1UL << 7)
#define RESETS_PADS_BANK0_Msk       (1UL << 8)
#define RESETS_PADS_QSPI_Msk        (1UL << 9)
#define RESETS_PIO0_Msk             (1UL << 10)
#define RESETS_PIO1_Msk             (1UL << 11)
#define RESETS_PLL_SYS_Msk          (1UL << 12)
#define RESETS_PLL_USB_Msk          (1UL << 13)
#define RESETS_PWM_Msk              (1UL << 14)
#define RESETS_RTC_Msk              (1UL << 15)
#define RESETS_SPI0_Msk             (1UL << 16)
#define RESETS_SPI1_Msk             (1UL << 17)
#define RESETS_SYSCFG_Msk           (1UL << 18)
#define RESETS_SYSINFO_Msk          (1UL << 19)
#define RESETS_TBMAN_Msk            (1UL << 20)
#define RESETS_TIMER_Msk            (1UL << 21)
#define RESETS_UART0_Msk            (1UL << 22)
#define RESETS_UART1_Msk            (1UL << 23)
#define RESETS_USBCTRL_Msk          (1UL << 24)


static const uint32_t uart_resets_reset_mask[] = {
    RESETS_UART0_Msk,
    RESETS_UART1_Msk,
};


static const uint32_t i2c_resets_reset_mask[] = {
    RESETS_I2C0_Msk,
    RESETS_I2C1_Msk,
};

static const uint32_t spi_resets_reset_mask[] = {
    RESETS_SPI0_Msk,
    RESETS_SPI1_Msk,
};


static const uint32_t uart_resets_reset_done_mask[] = {
    RESETS_UART0_Msk,
    RESETS_UART1_Msk,
};

static const uint32_t i2c_resets_reset_done_mask[] = {
    RESETS_I2C0_Msk,
    RESETS_I2C1_Msk,
};

static const uint32_t spi_resets_reset_done_mask[] = {
    RESETS_SPI0_Msk,
    RESETS_SPI1_Msk,
};

#endif /* RESETS_REG_H */