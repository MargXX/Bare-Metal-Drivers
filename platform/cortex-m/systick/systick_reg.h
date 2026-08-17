/**
 * @file systick_reg.h
 * @brief SysTick register map and bit field definitions for the RP2040 (ARM Cortex-M0+)
 *
 * Reference: RP2040 Datasheet, Sections 2.4.4, 2.4.8, Tables 88-91
 */

#ifndef SYSTICK_REG_H
#define SYSTICK_REG_H

#include <stdint.h>

// Base address
#define PPB_BASE 0xe0000000


// SYST_CVR CURRENT field: 24-bit (bits[23:0]) per ARMv6-M (RP2040).
// ARMv7-M (STM32G4) defines CURRENT as the full 32 bits, but RELOAD
// is bits[23:0] on both architectures, so CURRENT never actually
// holds a value wider than 24 bits in practice. Confirmed against
// ARM DDI 0419E (ARMv6-M) and ARM DDI 0403E (ARMv7-M).

// Register pointers 
#define SYST_CSR     (*(volatile uint32_t *)(PPB_BASE + 0xE010UL)) // Control and Status
#define SYST_RVR     (*(volatile uint32_t *)(PPB_BASE + 0xE014UL)) // Reload Value
#define SYST_CVR     (*(volatile uint32_t *)(PPB_BASE + 0xE018UL)) // Current Value
#define SYST_CALIB   (*(volatile uint32_t *)(PPB_BASE + 0xE01CUL)) // Calibration Value

// SYST_CSR bit masks
#define SYST_CSR_ENABLE_Msk         (1UL << 0)
#define SYST_CSR_TICKINT_Msk        (1UL << 1)
#define SYST_CSR_CLKSOURCE_Msk      (1UL << 2)
#define SYST_CSR_COUNTFLAG_Msk      (1UL << 16)    // read-only

// SYST_RVR bit mask, 0-23
#define SYST_RVR_RELOAD_Msk ((1UL << 24) - 1)

// SYST_CVR bit mask 
#define SYST_CVR_CURRENT_Msk ((1UL << 24) - 1)

// SYST_CALIB bit masks 
#define SYST_CALIB_TENMS_Msk    ((1UL << 24) - 1)   // read-only
#define SYST_CALIB_SKEW_Msk     (1UL << 30)         // read-only
#define SYST_CALIB_NOREF_Msk    (1UL << 31)         // read-only

#endif /* SYSTICK_REG_H */