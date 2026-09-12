#include "rcc_clock_init.h"
#include "stm32g431xx.h"


#define RCC_INIT_DELAY_CYCLE_COUNT 8UL
//reasonable time for 16 MHz, ~16.7s
#define RCC_INIT_TIMEOUT_CYCLES 0x0FFFFFFFUL

#define BM_RCC_PLL_M  4UL   // 16 MHz / 4 = 4 MHz PLL input
#define BM_RCC_PLL_N  72UL  // 4 MHz × 72 = 288 MHz VCO
#define BM_RCC_PLL_R  2UL   // 288 MHz / 2 = 144 MHz SYSCLK


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

    //prescalers left at reset values, which are fine for 144 MHz SYSCLK:
    //AHB prescaler = 1, APB1 prescaler = 1, APB2 prescaler = 1S

    SystemCoreClockUpdate(); // update SystemCoreClock variable to reflect new SYSCLK frequency

    return true;
}