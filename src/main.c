#include <stdint.h>

#include "lcd.h"
#include "red_pill.h"
#include "stm32f10x.h"

void delay(uint8_t mode) {
    TIM1->CCER = 0x1U << 0;
    TIM1->CCMR2 = 0x30;

    switch (mode) {
        case 1:  // 148Hz
            TIM1->ARR = 34 - 1;
            TIM1->PSC = 7200 - 1;
            TIM1->CCR1 = 17 - 1;
            break;
        case 2:  // 132Hz
            TIM1->ARR = 38 - 1;
            TIM1->PSC = 7200 - 1;
            TIM1->CCR1 = 19 - 1;
            break;
    }

    TIM1->CNT = 0;
    TIM1->SR = 0;
    TIM1->CR1 = 1;

    while ((TIM1->SR & 0x1U) == 0) {
    }

    TIM1->CR1 = 0;
}

void main() {
    RCC->APB2ENR = RCC_APB2ENR_AFIOEN | RCC_APB2ENR_IOPBEN | RCC_APB2ENR_TIM1EN;
    AFIO->MAPR |= AFIO_MAPR_SWJ_CFG_JTAGDISABLE;

    GPIOB->CRH = 0x44444444;
    GPIOB->CRL = 0x44444443;

    GPIOB->ODR = 0x0;

    while (1) {
        if (!(GPIOB->IDR & RPB_SW1)) {
            GPIOB->ODR ^= RPB_BUZZ;
            delay(2);
        } else if (!(GPIOB->IDR & RPB_SW2)) {
            GPIOB->ODR ^= RPB_BUZZ;
            delay(1);
        }
    }
}