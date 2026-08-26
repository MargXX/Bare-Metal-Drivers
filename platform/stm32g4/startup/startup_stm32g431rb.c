#include <stdint.h>

/* linker script symbols defined in the linker script stm32g431rb.ld */
extern uint32_t _sidata;
extern uint32_t _sdata;
extern uint32_t _edata;
extern uint32_t _sbss;
extern uint32_t _ebss;
extern uint32_t _estack;
extern int main(void);

void Reset_Handler(void)
{
    
    uint32_t *src = &_sidata;
    uint32_t *dst = &_sdata;

    //copy data from src to dst
    while (dst < &_edata) {
        *dst++ = *src++;
    }
    //zeroe bss
    dst = &_sbss;
    while (dst < &_ebss) {
        *dst++ = 0;
    }

    //if main accidentally returns, loop forever
    main();

    while (1) { }
}

void Default_Handler(void)
{
    while (1) { }
}

//weak sets low priority for the handlers, so that they can be overridden by user-defined handlers(like those in my drivers)
//alias sets the code to be Default_Handler's until overridden by user-defined handlers


/* core exceptions, ARMv7-M B1.5.2 Table B1-4 */
void NMI_Handler(void) __attribute__((weak, alias("Default_Handler")));
void HardFault_Handler(void) __attribute__((weak, alias("Default_Handler")));
void MemManage_Handler(void) __attribute__((weak, alias("Default_Handler")));
void BusFault_Handler(void) __attribute__((weak, alias("Default_Handler")));
void UsageFault_Handler(void) __attribute__((weak, alias("Default_Handler")));
void SVCall_Handler(void) __attribute__((weak, alias("Default_Handler")));
void DebugMonitor_Handler(void) __attribute__((weak, alias("Default_Handler")));
void PendSV_Handler(void) __attribute__((weak, alias("Default_Handler")));
void SysTick_Handler(void) __attribute__((weak, alias("Default_Handler")));

/* device-specific IRQs, verified against ST's official startup_stm32g431xx.s
   (STM32CubeG4 repo, NUCLEO-G431KB EWARM template), not RM0440's family-wide table */
void WWDG_IRQHandler(void) __attribute__((weak, alias("Default_Handler")));
void PVD_PVM_IRQHandler(void) __attribute__((weak, alias("Default_Handler")));
void RTC_TAMP_LSECSS_IRQHandler(void) __attribute__((weak, alias("Default_Handler")));
void RTC_WKUP_IRQHandler(void) __attribute__((weak, alias("Default_Handler")));
void FLASH_IRQHandler(void) __attribute__((weak, alias("Default_Handler")));
void RCC_IRQHandler(void) __attribute__((weak, alias("Default_Handler")));
void EXTI0_IRQHandler(void) __attribute__((weak, alias("Default_Handler")));
void EXTI1_IRQHandler(void) __attribute__((weak, alias("Default_Handler")));
void EXTI2_IRQHandler(void) __attribute__((weak, alias("Default_Handler")));
void EXTI3_IRQHandler(void) __attribute__((weak, alias("Default_Handler")));
void EXTI4_IRQHandler(void) __attribute__((weak, alias("Default_Handler")));
void DMA1_CH1_IRQHandler(void) __attribute__((weak, alias("Default_Handler")));
void DMA1_CH2_IRQHandler(void) __attribute__((weak, alias("Default_Handler")));
void DMA1_CH3_IRQHandler(void) __attribute__((weak, alias("Default_Handler")));
void DMA1_CH4_IRQHandler(void) __attribute__((weak, alias("Default_Handler")));
void DMA1_CH5_IRQHandler(void) __attribute__((weak, alias("Default_Handler")));
void DMA1_CH6_IRQHandler(void) __attribute__((weak, alias("Default_Handler")));
void ADC1_2_IRQHandler(void) __attribute__((weak, alias("Default_Handler")));
void USB_HP_IRQHandler(void) __attribute__((weak, alias("Default_Handler")));
void USB_LP_IRQHandler(void) __attribute__((weak, alias("Default_Handler")));
void FDCAN1_INTR0_IT_IRQHandler(void) __attribute__((weak, alias("Default_Handler")));
void FDCAN1_INTR1_IT_IRQHandler(void) __attribute__((weak, alias("Default_Handler")));
void EXTI9_5_IRQHandler(void) __attribute__((weak, alias("Default_Handler")));
void TIM1_BRK_TIM15_IRQHandler(void) __attribute__((weak, alias("Default_Handler")));
void TIM1_UP_TIM16_IRQHandler(void) __attribute__((weak, alias("Default_Handler")));
void TIM1_TRG_COM_TIM17_IRQHandler(void) __attribute__((weak, alias("Default_Handler")));
void TIM1_CC_IRQHandler(void) __attribute__((weak, alias("Default_Handler")));
void TIM2_IRQHandler(void) __attribute__((weak, alias("Default_Handler")));
void TIM3_IRQHandler(void) __attribute__((weak, alias("Default_Handler")));
void TIM4_IRQHandler(void) __attribute__((weak, alias("Default_Handler")));
void I2C1_EV_IRQHandler(void) __attribute__((weak, alias("Default_Handler")));
void I2C1_ER_IRQHandler(void) __attribute__((weak, alias("Default_Handler")));
void I2C2_EV_IRQHandler(void) __attribute__((weak, alias("Default_Handler")));
void I2C2_ER_IRQHandler(void) __attribute__((weak, alias("Default_Handler")));
void SPI1_IRQHandler(void) __attribute__((weak, alias("Default_Handler")));
void SPI2_IRQHandler(void) __attribute__((weak, alias("Default_Handler")));
void USART1_IRQHandler(void) __attribute__((weak, alias("Default_Handler")));
void USART2_IRQHandler(void) __attribute__((weak, alias("Default_Handler")));
void USART3_IRQHandler(void) __attribute__((weak, alias("Default_Handler")));
void EXTI15_10_IRQHandler(void) __attribute__((weak, alias("Default_Handler")));
void RTC_ALARM_IRQHandler(void) __attribute__((weak, alias("Default_Handler")));
void USBWakeUP_IRQHandler(void) __attribute__((weak, alias("Default_Handler")));
void TIM8_BRK_TERR_IERR_IRQHandler(void) __attribute__((weak, alias("Default_Handler")));
void TIM8_UP_IRQHandler(void) __attribute__((weak, alias("Default_Handler")));
void TIM8_TRG_COM_DIR_IDX_IRQHandler(void) __attribute__((weak, alias("Default_Handler")));
void TIM8_CC_IRQHandler(void) __attribute__((weak, alias("Default_Handler")));
void LPTIM1_IRQHandler(void) __attribute__((weak, alias("Default_Handler")));
void SPI3_IRQHandler(void) __attribute__((weak, alias("Default_Handler")));
void UART4_IRQHandler(void) __attribute__((weak, alias("Default_Handler")));
void TIM6_DAC_IRQHandler(void) __attribute__((weak, alias("Default_Handler")));
void TIM7_IRQHandler(void) __attribute__((weak, alias("Default_Handler")));
void DMA2_CH1_IRQHandler(void) __attribute__((weak, alias("Default_Handler")));
void DMA2_CH2_IRQHandler(void) __attribute__((weak, alias("Default_Handler")));
void DMA2_CH3_IRQHandler(void) __attribute__((weak, alias("Default_Handler")));
void DMA2_CH4_IRQHandler(void) __attribute__((weak, alias("Default_Handler")));
void DMA2_CH5_IRQHandler(void) __attribute__((weak, alias("Default_Handler")));
void UCPD1_IRQHandler(void) __attribute__((weak, alias("Default_Handler")));
void COMP1_2_3_IRQHandler(void) __attribute__((weak, alias("Default_Handler")));
void COMP4_IRQHandler(void) __attribute__((weak, alias("Default_Handler")));
void CRS_IRQHandler(void) __attribute__((weak, alias("Default_Handler")));
void SAI1_IRQHandler(void) __attribute__((weak, alias("Default_Handler")));
void FPU_IRQHandler(void) __attribute__((weak, alias("Default_Handler")));
void RNG_IRQHandler(void) __attribute__((weak, alias("Default_Handler")));
void LPUART1_IRQHandler(void) __attribute__((weak, alias("Default_Handler")));
void I2C3_EV_IRQHandler(void) __attribute__((weak, alias("Default_Handler")));
void I2C3_ER_IRQHandler(void) __attribute__((weak, alias("Default_Handler")));
void DMAMUX_OVR_IRQHandler(void) __attribute__((weak, alias("Default_Handler")));
void DMA2_CH6_IRQHandler(void) __attribute__((weak, alias("Default_Handler")));
void CORDIC_IRQHandler(void) __attribute__((weak, alias("Default_Handler")));
void FMAC_IRQHandler(void) __attribute__((weak, alias("Default_Handler")));

__attribute__((section(".isr_vector")))
void (* const g_pfnVectors[])(void) = {
    (void (*)(void))&_estack,      /* 0  initial SP */
    Reset_Handler,                 /* 1 */
    NMI_Handler,                   /* 2 */
    HardFault_Handler,             /* 3 */
    MemManage_Handler,             /* 4 */
    BusFault_Handler,              /* 5 */
    UsageFault_Handler,            /* 6 */
    0, 0, 0, 0,                    /* 7-10 reserved */
    SVCall_Handler,                /* 11 */
    DebugMonitor_Handler,          /* 12 */
    0,                              /* 13 reserved */
    PendSV_Handler,                 /* 14 */
    SysTick_Handler,                /* 15 */

    /* device-specific IRQs, vector 16-117, IRQ0-101 per startup_stm32g431xx.s */
    WWDG_IRQHandler,
    PVD_PVM_IRQHandler,
    RTC_TAMP_LSECSS_IRQHandler,
    RTC_WKUP_IRQHandler,
    FLASH_IRQHandler,
    RCC_IRQHandler,
    EXTI0_IRQHandler,
    EXTI1_IRQHandler,
    EXTI2_IRQHandler,
    EXTI3_IRQHandler,
    EXTI4_IRQHandler,
    DMA1_CH1_IRQHandler,
    DMA1_CH2_IRQHandler,
    DMA1_CH3_IRQHandler,
    DMA1_CH4_IRQHandler,
    DMA1_CH5_IRQHandler,
    DMA1_CH6_IRQHandler,
    0,                              /* IRQ17 reserved */
    ADC1_2_IRQHandler,
    USB_HP_IRQHandler,
    USB_LP_IRQHandler,
    FDCAN1_INTR0_IT_IRQHandler,
    FDCAN1_INTR1_IT_IRQHandler,
    EXTI9_5_IRQHandler,
    TIM1_BRK_TIM15_IRQHandler,
    TIM1_UP_TIM16_IRQHandler,
    TIM1_TRG_COM_TIM17_IRQHandler,
    TIM1_CC_IRQHandler,
    TIM2_IRQHandler,
    TIM3_IRQHandler,
    TIM4_IRQHandler,
    I2C1_EV_IRQHandler,
    I2C1_ER_IRQHandler,
    I2C2_EV_IRQHandler,
    I2C2_ER_IRQHandler,
    SPI1_IRQHandler,
    SPI2_IRQHandler,
    USART1_IRQHandler,
    USART2_IRQHandler,
    USART3_IRQHandler,
    EXTI15_10_IRQHandler,
    RTC_ALARM_IRQHandler,
    USBWakeUP_IRQHandler,
    TIM8_BRK_TERR_IERR_IRQHandler,
    TIM8_UP_IRQHandler,
    TIM8_TRG_COM_DIR_IDX_IRQHandler,
    TIM8_CC_IRQHandler,
    0, 0,                           /* IRQ47-48 reserved */
    LPTIM1_IRQHandler,
    0,                              /* IRQ50 reserved */
    SPI3_IRQHandler,
    UART4_IRQHandler,
    0,                              /* IRQ53 reserved */
    TIM6_DAC_IRQHandler,
    TIM7_IRQHandler,
    DMA2_CH1_IRQHandler,
    DMA2_CH2_IRQHandler,
    DMA2_CH3_IRQHandler,
    DMA2_CH4_IRQHandler,
    DMA2_CH5_IRQHandler,
    0, 0,                           /* IRQ61-62 reserved */
    UCPD1_IRQHandler,
    COMP1_2_3_IRQHandler,
    COMP4_IRQHandler,
    0, 0, 0, 0, 0, 0, 0, 0, 0,      /* IRQ66-74 reserved */
    CRS_IRQHandler,
    SAI1_IRQHandler,
    0, 0, 0, 0,                     /* IRQ77-80 reserved */
    FPU_IRQHandler,
    0, 0, 0, 0, 0, 0, 0, 0,         /* IRQ82-89 reserved */
    RNG_IRQHandler,
    LPUART1_IRQHandler,
    I2C3_EV_IRQHandler,
    I2C3_ER_IRQHandler,
    DMAMUX_OVR_IRQHandler,
    0, 0,                           /* IRQ95-96 reserved */
    DMA2_CH6_IRQHandler,
    0, 0,                           /* IRQ98-99 reserved */
    CORDIC_IRQHandler,
    FMAC_IRQHandler,
};