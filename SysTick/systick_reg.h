/**
 * @file systick_reg.h
 * @brief SysTick register map and bit field definitions for the RP2040 (ARM Cortex-M0+)
 *
 * Reference: RP2040 Datasheet, Sections 2.4.4, 2.4.8, Tables 88-91
 */

#ifndef SYSTICK_REG_H
#define SYSTICK_REG_H

#include <stdint.h>

// Base address — find in Section 2.4.8
#define PPB_BASE

// Register pointers — find offsets in Section 2.4.8
#define SYST_CSR    // Control and Status
#define SYST_RVR    // Reload Value
#define SYST_CVR    // Current Value
#define SYST_CALIB  // Calibration Value

// SYST_CSR bit masks — Table 88, three writable bits and one read-only
#define SYST_CSR_ENABLE_Msk
#define SYST_CSR_TICKINT_Msk
#define SYST_CSR_CLKSOURCE_Msk
#define SYST_CSR_COUNTFLAG_Msk      // read-only

// SYST_RVR bit mask — Table 89, how many bits wide is the reload value?
#define SYST_RVR_RELOAD_Msk

// SYST_CVR bit mask — Table 90, same width as RVR
#define SYST_CVR_CURRENT_Msk

// SYST_CALIB bit masks — Table 91, all read-only
#define SYST_CALIB_TENMS_Msk        // read-only
#define SYST_CALIB_SKEW_Msk         // read-only
#define SYST_CALIB_NOREF_Msk        // read-only

#endif /* SYSTICK_REG_H */