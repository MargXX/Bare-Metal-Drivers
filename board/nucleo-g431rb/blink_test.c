#include "stm32g431xx.h"

int main(void) {
    // enable gpio clock
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


    while(1) {
        //toggle PA5 on
        GPIOA->BSRR = GPIO_BSRR_BS5_Msk;
        
        //delaya
        for (volatile int i = 0; i < 1000000; i++);

        GPIOA->BSRR = GPIO_BSRR_BR5_Msk; // reset PA5

        //delay
        for (volatile int i = 0; i < 1000000; i++);
    }
}