#include <stdint.h>

#include "lcd.h"
#include "red_pill.h"
#include "stm32f10x.h"

void delay(uint8_t mode) {
    TIM3->CCER = 0x1U << 8;
    TIM3->CCMR2 = 0x30;

    TIM3->PSC = 200 - 1;

    switch (mode) {
        case 1:  // 25%
            TIM3->ARR = 20;
            break;
        case 2:  // 50%
            TIM3->ARR = 40;
            break;
        case 3:  // 75%
            TIM3->ARR = 60;
            break;
    }

    TIM3->CCR3 = TIM3->ARR / 2;

    TIM3->CNT = 0;
    TIM3->SR = 0;
    TIM3->CR1 = 1;

    while ((TIM3->SR & 0x1U) == 0) {
    }

    TIM3->CR1 = 0;
}

void main() {
    RCC->APB1ENR = RCC_APB1ENR_TIM3EN;
    RCC->APB2ENR = RCC_APB2ENR_AFIOEN | RCC_APB2ENR_IOPBEN;
    AFIO->MAPR |= AFIO_MAPR_SWJ_CFG_JTAGDISABLE;

    GPIOB->CRH = 0x44444444;
    GPIOB->CRL = 0x44444443;

    GPIOB->ODR = 0x0;

    while (1) {
        // switch (!GPIOB->IDR) {
        //     case RPB_SW1:
        //         GPIOB->ODR ^= RPB_BUZZ;
        //         delay(1);
        //         break;
        //     case RPB_SW2:
        //         GPIOB->ODR ^= RPB_BUZZ;
        //         delay(2);
        //         break;
        //     case RPB_SW3:
        //         GPIOB->ODR ^= RPB_BUZZ;
        //         delay(3);
        //         break;
        //     default:
        //         GPIOB->ODR &= 0xFFFF & !RPB_BUZZ;
        //         break;
        // }

        if (!(GPIOB->IDR & RPB_SW1)) {
            GPIOB->ODR ^= RPB_BUZZ;
            delay(1);
        } else if (!(GPIOB->IDR & RPB_SW2)) {
            GPIOB->ODR ^= RPB_BUZZ;
            delay(2);
        } else if (!(GPIOB->IDR & RPB_SW3)) {
            GPIOB->ODR ^= RPB_BUZZ;
            delay(3);
        } else {
            GPIOB->ODR &= 0xFFFF & !RPB_BUZZ;
        }
    }
}