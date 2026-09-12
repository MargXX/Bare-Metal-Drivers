#include "rcc.h"
#include "stm32g431xx.h"


/**
 * Does not configure PWR_CR1 (VOS) or PWR_CR5 (R1MODE): both reset
 * values already satisfy Range 1 normal mode at this target frequency, given they reset to VOS = 1 (Range 1) and R1MODE = 1 (normal mode). 
 * verified against RM0440 register descriptions.
 */

#define RCC_INIT_TIMEOUT_CYCLES 0x00FFFFFFUL

#define BM_RCC_PLL_M  4UL   // 16 MHz / 4 = 4 MHz PLL input
#define BM_RCC_PLL_N  72UL  // 4 MHz × 72 = 288 MHz VCO
#define BM_RCC_PLL_R  2UL   // 288 MHz / 2 = 144 MHz SYSCLK

#define BM_RCC_HSI_HZ 16000000UL

uint32_t SystemCoreClock = BM_RCC_HSI_HZ; // default reset value, updated by bm_rcc_clock_init() if successful


_Static_assert((BM_RCC_HSI_HZ * BM_RCC_PLL_N) / (BM_RCC_PLL_M * BM_RCC_PLL_R)
               == BM_RCC_SYSCLK_HZ, "PLL config disagrees with advertised SYSCLK");

    

bool bm_rcc_clock_init(void) {
    uint32_t timeout = RCC_INIT_TIMEOUT_CYCLES;

    // configure wait states for 144 MHz SYSCLK
    FLASH->ACR = (FLASH->ACR & ~FLASH_ACR_LATENCY_Msk) | (4UL << FLASH_ACR_LATENCY_Pos);
    //check that the wait states were set correctly
    if ((FLASH->ACR & FLASH_ACR_LATENCY_Msk) != (4UL << FLASH_ACR_LATENCY_Pos)) {
        return false;
    }

    // enable HS116 CLock
    RCC->CR |= RCC_CR_HSION_Msk; // enable HSI16
    // wait for HSI16 ready
    timeout = RCC_INIT_TIMEOUT_CYCLES;
    while (!(RCC->CR & RCC_CR_HSIRDY_Msk)) {
        if (timeout-- == 0) { return false;}
    } 
    
    // PLL
    // disable PLL before reconfiguring
    RCC->CR &= ~RCC_CR_PLLON_Msk;
    // wait for PLL ready
    timeout = RCC_INIT_TIMEOUT_CYCLES;
    while (RCC->CR & RCC_CR_PLLRDY_Msk) {// wait for PLL ready to clear
        if (timeout-- == 0) { return false; }
    }
    //set HSI16 as PLL source
    RCC->PLLCFGR = (RCC->PLLCFGR & ~RCC_PLLCFGR_PLLSRC_Msk) | (0b10UL << RCC_PLLCFGR_PLLSRC_Pos);
    //change PLL configuration to M=4, N=72, R=2
    RCC->PLLCFGR = (RCC->PLLCFGR & ~RCC_PLLCFGR_PLLM_Msk) | ((BM_RCC_PLL_M-1UL) << RCC_PLLCFGR_PLLM_Pos);
    RCC->PLLCFGR = (RCC->PLLCFGR & ~RCC_PLLCFGR_PLLN_Msk) | (BM_RCC_PLL_N << RCC_PLLCFGR_PLLN_Pos);
    // RM0440 RCC_PLLCFGR: PLLR field 00 = divide by 2
    RCC->PLLCFGR = (RCC->PLLCFGR & ~RCC_PLLCFGR_PLLR_Msk) | (0UL << RCC_PLLCFGR_PLLR_Pos);
    RCC->PLLCFGR = (RCC->PLLCFGR | RCC_PLLCFGR_PLLREN_Msk); // enable PLLR output
    // NOTE: TODO: adjust PLLQ when I add USB support, to 6 (48 MHz), leaving unfinished now to save power.
    //enable PLL again
    RCC->CR |= RCC_CR_PLLON_Msk;
    // wait for PLL ready
    timeout = RCC_INIT_TIMEOUT_CYCLES;
    while (!(RCC->CR & RCC_CR_PLLRDY_Msk)) {
        if (timeout-- == 0) { return false; }
    }
    // switch SYSCLK to PLL
    RCC->CFGR = (RCC->CFGR & ~RCC_CFGR_SW_Msk) | (0b11UL << RCC_CFGR_SW_Pos);
    // wait for SYSCLK switch to PLL
    timeout = RCC_INIT_TIMEOUT_CYCLES;
    while ((RCC->CFGR & RCC_CFGR_SWS_Msk) != (0b11UL << RCC_CFGR_SWS_Pos)) {
        if (timeout-- == 0) { return false; }
    }

    //prescalers left at reset values, which are fine for 144 MHz
    //RM0440 Table 17. Reset values for RCC_CFGR register: HPRE = 0b0000 (SYSCLK not divided), PPRE1 = 0b000 (HCLK not divided), PPRE2 = 0b000 (HCLK not divided)
    //prescalers are /1 so HCLK = PCLK1 = PCLK2 = SYSCLK = 144 MHz

    // update SystemCoreClock variable to reflect new SYSCLK frequency
    SystemCoreClock = BM_RCC_SYSCLK_HZ;

               

    return true;
}