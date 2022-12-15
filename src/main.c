#include <stdbool.h>
#include <stdint.h>

#include "lcd.h"
#include "red_pill.h"
#include "stm32f10x.h"

bool isFirstOctave = true;
bool isSwitchPressed = false;

void configureTimers() {
    // enable timers' CCER channels which will be needed
    TIM1->CCER = TIM_CCER_CC1E | TIM_CCER_CC2E | TIM_CCER_CC3E | TIM_CCER_CC4E;
    TIM2->CCER = TIM_CCER_CC1E | TIM_CCER_CC2E | TIM_CCER_CC3E | TIM_CCER_CC4E;
    TIM3->CCER = TIM_CCER_CC1E | TIM_CCER_CC2E | TIM_CCER_CC3E | TIM_CCER_CC4E;
    TIM4->CCER = TIM_CCER_CC1E | TIM_CCER_CC2E;

    // set all timers' channels to PWM mode
    TIM1->CCMR1 = TIM_CCMR1_OC1M | TIM_CCMR1_OC2M;
    TIM1->CCMR2 = TIM_CCMR2_OC3M | TIM_CCMR2_OC4M;

    TIM2->CCMR1 = TIM_CCMR1_OC1M | TIM_CCMR1_OC2M;
    TIM2->CCMR2 = TIM_CCMR2_OC3M | TIM_CCMR2_OC4M;

    TIM3->CCMR1 = TIM_CCMR1_OC1M | TIM_CCMR1_OC2M;
    TIM3->CCMR2 = TIM_CCMR2_OC3M | TIM_CCMR2_OC4M;

    TIM4->CCMR1 = TIM_CCMR1_OC1M | TIM_CCMR1_OC2M;

    // configure prescaler for all timers
    TIM1->PSC = 72;
    TIM2->PSC = 72;
    TIM3->PSC = 72;
    TIM4->PSC = 72;

    TIM1->CCR1 = 0;
    TIM1->CCR2 = 0;
    TIM1->CCR3 = 0;
    TIM1->CCR4 = 0;
    TIM2->CCR1 = 0;
    TIM2->CCR2 = 0;
    TIM2->CCR3 = 0;
    TIM2->CCR4 = 0;
    TIM3->CCR1 = 0;
    TIM3->CCR2 = 0;
    TIM3->CCR3 = 0;
    TIM3->CCR4 = 0;
    TIM4->CCR1 = 0;
    TIM4->CCR2 = 0;

    TIM1->CNT = 0;
    TIM2->CNT = 0;
    TIM3->CNT = 0;
    TIM4->CNT = 0;
}

void engageTimer(uint8_t note) {
    switch (note) {
        case 1:  // 132Hz
            TIM2->ARR = 3788;
            TIM2->CCR2 = 3788 / 2;
            TIM2->SR = 0;
            TIM2->CR1 = 1;

            while ((TIM2->SR & 0x1U) == 0)
                ;

            TIM2->CR1 = 0;
            break;
        case 2:  // 140Hz
            TIM2->ARR = 3572;
            TIM2->CCR3 = 3572 / 2;
            TIM2->SR = 0;
            TIM2->CR1 = 1;

            while ((TIM2->SR & 0x1U) == 0)
                ;

            TIM2->CR1 = 0;
            break;
        case 3:  // 148Hz
            TIM3->ARR = 3378;
            TIM3->CCR1 = 3378 / 2;
            TIM3->SR = 0;
            TIM3->CR1 = 1;

            while ((TIM3->SR & 0x1U) == 0)
                ;

            TIM3->CR1 = 0;
            break;
        case 4:  // 157Hz
            TIM3->ARR = 3186;
            TIM3->CCR2 = 3186 / 2;
            TIM3->SR = 0;
            TIM3->CR1 = 1;

            while ((TIM3->SR & 0x1U) == 0)
                ;

            TIM3->CR1 = 0;
            break;
        case 5:  // 166Hz
            TIM2->ARR = 3012;
            TIM2->CCR2 = 3012 / 2;
            TIM2->SR = 0;
            TIM2->CR1 = 1;

            while ((TIM2->SR & 0x1U) == 0)
                ;

            TIM2->CR1 = 0;
            break;
        case 6:  // 176Hz
            TIM2->ARR = 2842;
            TIM2->CCR4 = 2842 / 2;
            TIM2->SR = 0;
            TIM2->CR1 = 1;

            while ((TIM2->SR & 0x1U) == 0)
                ;

            TIM2->CR1 = 0;
            break;
    }
}

void buzz(uint8_t note) {
    isSwitchPressed = true;

    GPIOB->ODR ^= RPB_BUZZ;

    if (isFirstOctave) {
        engageTimer(note);
    } else {
        engageTimer(note + 13);
    }
}

void main(void) {
    uint32_t inputDataA, inputDataB, inputDataC;

    RCC->APB1ENR = RCC_APB1ENR_TIM2EN | RCC_APB1ENR_TIM3EN | RCC_APB1ENR_TIM4EN;
    RCC->APB2ENR =
        RCC_APB2ENR_AFIOEN | RCC_APB2ENR_IOPAEN | RCC_APB2ENR_IOPBEN | RCC_APB2ENR_IOPCEN | RCC_APB2ENR_TIM1EN;
    AFIO->MAPR |= AFIO_MAPR_SWJ_CFG_JTAGDISABLE;

    GPIOA->CRH = 0x34433443;
    GPIOA->CRL = 0x43344444;

    GPIOB->CRH = 0x44444444;
    GPIOB->CRL = 0x44444443;

    GPIOC->CRH = 0x44444444;
    GPIOC->CRL = 0x44444444;

    GPIOB->ODR = 0x0;

    configureTimers();

    while (1) {
        inputDataA = ~GPIOA->IDR & (RPA_SW8 | RPA_SW9 | RPA_SW14);
        inputDataB = ~GPIOB->IDR & (RPB_SW1 | RPB_SW2 | RPB_SW3 | RPB_SW4 | RPB_SW5 | RPB_SW6 | RPB_SW7 | RPB_SW10 |
                                    RPB_SW11 | RPB_SW12 | RPB_SW13);
        inputDataC = ~GPIOC->IDR & (RPC_SW15 | RPC_SW16 | RPC_SW17);

        switch (inputDataA) {
            case RPA_SW14:
                buzz(4);
                break;
            case RPA_SW8:
                buzz(6);
                break;
            default:
                isSwitchPressed = false;
                break;
        }

        switch (inputDataB) {
            case RPB_SW5:
                buzz(1);
                break;
            case RPB_SW13:
                buzz(2);
                break;
            case RPB_SW6:
                buzz(3);
                break;
            case RPB_SW7:
                buzz(5);
                break;
            default:
                isSwitchPressed = false;
                break;
        }

        if (!isSwitchPressed) {
            GPIOB->ODR &= 0xFFFFFFFF & !RPB_BUZZ;
        }
    }
}