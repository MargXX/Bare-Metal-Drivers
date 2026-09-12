#include "stm32g431xx.h"
#include "rcc.h"
#include "systick.h"

/*
 * clock_test — hardware verification for bm_rcc_clock_init() and bm_systick_init().
 *
 * PA5 (LD2): 1 Hz blink driven by bm_systick_delay_ms(). Coarse check —
 *   confirms the SYSCLK switch happened and the tick interrupt fires.
 *   A ~9x slow blink means the core is still on HSI16 at 16 MHz while the
 *   SysTick reload is sized for 144 MHz.
 * PA8 (MCO): SYSCLK routed out at /16, nominally 9 MHz. Direct frequency
 *   measurement with a scope — this is what actually verifies 144 MHz.
 *   Stopwatch timing on the LED cannot resolve better than ~1%.
 *
 * On init failure: fast busy-loop blink on PA5, no SysTick dependency.
 */

/* MCO: RM0440 RCC_CFGR. MCOSEL = 0b0001 (SYSCLK), MCOPRE = 0b100 (/16).
   144 MHz / 16 = 9 MHz at PA8. Verify both encodings against RM0440
   before trusting the measurement. */
#define MCO_SEL_SYSCLK  0b0001UL
#define MCO_PRE_DIV16   0b100UL

int main(void) {
    // enable gpio clock — done before clock init so the failure path can blink
    RCC->AHB2ENR |= RCC_AHB2ENR_GPIOAEN;
    (void)RCC->AHB2ENR;   /* RM0440: enable bit needs a read-back before access */

    //configure PA5 as output
    GPIOA->MODER &= ~(GPIO_MODER_MODE5_Msk); // clear mode bits
    GPIOA->MODER |= (1 << GPIO_MODER_MODE5_Pos); // set mode to output

    GPIOA->OTYPER &= ~(GPIO_OTYPER_OT5_Msk); // clear output type bit for PA5

    //no pull
    GPIOA->PUPDR &= ~(GPIO_PUPDR_PUPD5_Msk); // clear pull-up/pull-down bits for PA5
    //low speed
    GPIOA->OSPEEDR &= ~(GPIO_OSPEEDR_OSPEED5_Msk); // clear speed bits for PA5

    // && not |, and short-circuits so SysTick never inits against a failed clock
    bool result = bm_rcc_clock_init();   // initialize system clock to 144 MHz
    result = result && bm_systick_init();

    if (!result) {
        // fast blink, busy-loop only — does not depend on SysTick or the PLL
        while (1) {
            GPIOA->BSRR = GPIO_BSRR_BS5_Msk;
            for (volatile int i = 0; i < 100000; i++);
            GPIOA->BSRR = GPIO_BSRR_BR5_Msk;
            for (volatile int i = 0; i < 100000; i++);
        }
    }

    //configure PA8 as alternate function for MCO output
    GPIOA->MODER &= ~(GPIO_MODER_MODE8_Msk); // clear mode bits
    GPIOA->MODER |= (2 << GPIO_MODER_MODE8_Pos); // 0b10 = alternate function

    GPIOA->OTYPER &= ~(GPIO_OTYPER_OT8_Msk); // push-pull
    GPIOA->PUPDR &= ~(GPIO_PUPDR_PUPD8_Msk); // no pull

    // very high speed — at 9 MHz a slow slew rate rounds the edges badly
    GPIOA->OSPEEDR |= (3 << GPIO_OSPEEDR_OSPEED8_Pos);

    // AF0 = MCO on PA8. AFR[1] covers pins 8-15, so pin 8 is field 0.
    GPIOA->AFR[1] &= ~(GPIO_AFRH_AFSEL8_Msk); // 0b0000 = AF0

    //route SYSCLK to MCO at /16
    RCC->CFGR &= ~(RCC_CFGR_MCOSEL_Msk | RCC_CFGR_MCOPRE_Msk);
    RCC->CFGR |= (MCO_SEL_SYSCLK << RCC_CFGR_MCOSEL_Pos)
               | (MCO_PRE_DIV16 << RCC_CFGR_MCOPRE_Pos);

    while(1) {
        //toggle PA5 on
        GPIOA->BSRR = GPIO_BSRR_BS5_Msk;

        //delay
        bm_systick_delay_ms(1000); // delay 1 second

        GPIOA->BSRR = GPIO_BSRR_BR5_Msk; // reset PA5

        //delay
        bm_systick_delay_ms(500); // delay 1 second
    }
}