/**
 * @file gpio_reg.h
 * @brief GPIO register map, pin constants, and function definitions for the RP2040
 *
 * Reference: RP2040 Datasheet, Sections 2.3 (SIO), 2.19 (GPIO)
 */

#ifndef GPIO_REG_H
#define GPIO_REG_H

#include <stdint.h>

//GPIO pin registers come in pairs: status/ctrl
typedef struct {  
    uint32_t status;
    uint32_t ctrl;
} gpio_pin_regs_t;
//8 bytes per pin struct

//register pointer for the whole bank of GPIO pins, which we can index into with pin number to get to the right registers
// volitile tell compiler not to optimize access to these registers
#define IO_BANK0_BASE   0x40014000UL
#define SIO_BASE        0xD0000000UL

#define IO_BANK0    ((volatile gpio_pin_regs_t *)IO_BANK0_BASE)
#define SIO         ((volatile uint32_t *)SIO_BASE)



// SIO GPIO Register Offsets
#define GPIO_IN      1

#define GPIO_OUT     4
#define GPIO_OUT_SET 5
#define GPIO_OUT_CLR 6
#define GPIO_OUT_XOR 7

#define GPIO_OE      8
#define GPIO_OE_SET  9
#define GPIO_OE_CLR  10

#define IO_BANK_Msk  0b00011111


//IO_BAKN0 CTRL register function select values


//from datasheet section 2.1.2
// Addr + 0x0000 : normal read write access
// Addr + 0x1000 : atomic XOR on write
// Addr + 0x2000 : atomic bitmask set on write
// Addr + 0x3000 : atomic bitmask clear on write

#define REG_ALIAS_SET_OFFSET  0x2000u
#define REG_ALIAS_CLR_OFFSET  0x3000u

#define IO_BANK0_SET    ((volatile gpio_pin_regs_t *)(IO_BANK0_BASE + REG_ALIAS_SET_OFFSET))
#define IO_BANK0_CLR    ((volatile gpio_pin_regs_t *)(IO_BANK0_BASE + REG_ALIAS_CLR_OFFSET))



#endif /* GPIO_REG_H */