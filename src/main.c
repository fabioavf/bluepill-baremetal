#include <stdbool.h>
#include <stdint.h>

#include "lcd.h"
#include "red_pill.h"
#include "stm32f10x.h"

#define DEBOUNCE_TIMES 50  // number of times switch HI has to be read before counting as a press

bool isFirstOctave = true;
uint8_t pwmLoad = 1;
uint8_t debounceCounter = 0;
uint16_t adcValue = 0, adcValueMemory = 0;

void buzz(uint8_t note);
void configure_timers();
void cycle_pwm_load();
void engage_timer(uint8_t note);

uint16_t bin_bcd(uint16_t input) {
    uint8_t shift = 0;
    uint16_t output = 0;

    while (input > 0) {
        output |= (input % 10) << (shift++ << 2);
        input /= 10;
    }

    return output;
}

void main(void) {
    bool octaveSelectorMemory = isFirstOctave;
    uint32_t inputDataA, inputDataB, inputDataC, inputData;
    uint8_t pwmLoadMemory = pwmLoad;

    RCC->APB1ENR = RCC_APB1ENR_TIM2EN | RCC_APB1ENR_TIM3EN | RCC_APB1ENR_TIM4EN;
    RCC->APB2ENR = RCC_APB2ENR_AFIOEN | RCC_APB2ENR_IOPAEN | RCC_APB2ENR_IOPBEN | RCC_APB2ENR_IOPCEN |
                   RCC_APB2ENR_TIM1EN | RCC_APB2ENR_ADC1EN;
    AFIO->MAPR |= AFIO_MAPR_SWJ_CFG_JTAGDISABLE;

    GPIOA->CRH = 0x34433443;
    GPIOA->CRL = 0x43344444;

    GPIOB->CRH = 0x44444444;
    GPIOB->CRL = 0x44444403;

    GPIOC->CRH = 0x44444444;
    GPIOC->CRL = 0x44444444;

    GPIOB->ODR = 0x0;

    delay_us(10);

    ADC1->CR2 = 1;
    ADC1->SMPR2 = 0x1U << 3;

    configure_timers();
    delay_us(10);

    lcd_init();
    delay_us(10);

    lcd_command(0x0c);
    lcd_print("Oitava: 1");
    lcd_command(0xc0);
    lcd_print("Load: 25%");

    while (1) {
        inputDataA = ~GPIOA->IDR & (RPA_SW8 | RPA_SW9 | RPA_SW14);
        inputDataB = ~GPIOB->IDR & (RPB_SW1 | RPB_SW2 | RPB_SW3 | RPB_SW4 | RPB_SW5 | RPB_SW6 | RPB_SW7 | RPB_SW10 |
                                    RPB_SW11 | RPB_SW12 | RPB_SW13);
        inputDataC = ~GPIOC->IDR & (RPC_SW15 | RPC_SW16 | RPC_SW17);

        inputData = inputDataA | inputDataB | inputDataC;

        ADC1->SQR3 = 9;
        ADC1->CR2 = 1;
        while ((ADC1->SR & (0x1U << 1)) == 0)
            ;
        adcValue = ADC1->DR;

        delay_us(10);

        if (isFirstOctave != octaveSelectorMemory) {
            if (debounceCounter > DEBOUNCE_TIMES) {
                lcd_command(0x88);
                lcd_data(isFirstOctave ? 0x31 : 0x32);
                octaveSelectorMemory = isFirstOctave;
                debounceCounter = 0;
            } else {
                debounceCounter++;
            }
        }

        if (pwmLoad != pwmLoadMemory) {
            if (debounceCounter > DEBOUNCE_TIMES) {
                lcd_command(0xc6);

                switch (pwmLoad) {
                    case 1:
                        lcd_print("25%");
                        break;
                    case 2:
                        lcd_print("50%");
                        break;
                    case 3:
                        lcd_print("75%");
                        break;
                }

                pwmLoadMemory = pwmLoad;
                debounceCounter = 0;
            } else {
                debounceCounter++;
            }
        }

        if (inputData == inputDataA) {
            switch (inputDataA) {
                case RPA_SW14:
                    buzz(4);
                    break;
                case RPA_SW8:
                    buzz(6);
                    break;
                case RPA_SW9:
                    buzz(8);
                    break;
                default:
                    GPIOB->ODR &= 0xFFFFFFFF & !RPB_BUZZ;
                    adcValueMemory = adcValue;
                    break;
            }
        } else if (inputData == inputDataB) {
            switch (inputDataB) {
                case RPB_SW3:
                    cycle_pwm_load();
                    break;
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
                case RPB_SW10:
                    buzz(10);
                    break;
                case RPB_SW11:
                    buzz(12);
                    break;
                case RPB_SW12:
                    buzz(13);
                    break;
                case RPB_SW2:
                    isFirstOctave = !isFirstOctave;
                    break;
                default:
                    GPIOB->ODR &= 0xFFFFFFFF & !RPB_BUZZ;
                    adcValueMemory = adcValue;
                    break;
            }
        } else if (inputData == inputDataC) {
            switch (inputDataC) {
                case RPC_SW15:
                    buzz(7);
                    break;
                case RPC_SW16:
                    buzz(9);
                    break;
                case RPC_SW17:
                    buzz(11);
                    break;
                default:
                    GPIOB->ODR &= 0xFFFFFFFF & !RPB_BUZZ;
                    adcValueMemory = adcValue;
                    break;
            }
        }
    }
}

void configure_timers() {
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
    TIM4->CCMR2 = TIM_CCMR2_OC3M | TIM_CCMR2_OC4M;

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
    TIM4->CCR3 = 0;
    TIM4->CCR4 = 0;

    TIM1->CNT = 0;
    TIM2->CNT = 0;
    TIM3->CNT = 0;
    TIM4->CNT = 0;
}

void engage_timer(uint8_t note) {
    uint16_t distortion = (adcValueMemory - adcValue) / 4;

    switch (note) {
        case 1:  // 132Hz
            TIM2->ARR = 3788 + distortion;
            TIM2->CCR2 = (3788 / 4) * pwmLoad;
            TIM2->SR = 0;
            TIM2->CR1 = 1;

            while ((TIM2->SR & 0x1U) == 0)
                ;

            TIM2->CR1 = 0;
            break;
        case 2:  // 140Hz
            TIM2->ARR = 3572 + distortion;
            TIM2->CCR3 = (3572 / 4) * pwmLoad;
            TIM2->SR = 0;
            TIM2->CR1 = 1;

            while ((TIM2->SR & 0x1U) == 0)
                ;

            TIM2->CR1 = 0;
            break;
        case 3:  // 148Hz
            TIM3->ARR = 3378 + distortion;
            TIM3->CCR1 = (3378 / 4) * pwmLoad;
            TIM3->SR = 0;
            TIM3->CR1 = 1;

            while ((TIM3->SR & 0x1U) == 0)
                ;

            TIM3->CR1 = 0;
            break;
        case 4:  // 157Hz
            TIM3->ARR = 3186 + distortion;
            TIM3->CCR2 = (3186 / 4) * pwmLoad;
            TIM3->SR = 0;
            TIM3->CR1 = 1;

            while ((TIM3->SR & 0x1U) == 0)
                ;

            TIM3->CR1 = 0;
            break;
        case 5:  // 166Hz
            TIM2->ARR = 3012 + distortion;
            TIM2->CCR2 = (3012 / 4) * pwmLoad;
            TIM2->SR = 0;
            TIM2->CR1 = 1;

            while ((TIM2->SR & 0x1U) == 0)
                ;

            TIM2->CR1 = 0;
            break;
        case 6:  // 176Hz
            TIM2->ARR = 2842 + distortion;
            TIM2->CCR4 = (2842 / 4) * pwmLoad;
            TIM2->SR = 0;
            TIM2->CR1 = 1;

            while ((TIM2->SR & 0x1U) == 0)
                ;

            TIM2->CR1 = 0;
            break;
        case 7:  // 187Hz
            TIM3->ARR = 2674 + distortion;
            TIM3->CCR2 = (2674 / 4) * pwmLoad;
            TIM3->SR = 0;
            TIM3->CR1 = 1;

            while ((TIM3->SR & 0x1U) == 0)
                ;

            TIM3->CR1 = 0;
            break;
        case 8:  // 198Hz
            TIM2->ARR = 2526 + distortion;
            TIM2->CCR2 = (2526 / 4) * pwmLoad;
            TIM2->SR = 0;
            TIM2->CR1 = 1;

            while ((TIM2->SR & 0x1U) == 0)
                ;

            TIM2->CR1 = 0;
            break;
        case 9:  // 209Hz
            TIM3->ARR = 2392 + distortion;
            TIM3->CCR2 = (2392 / 4) * pwmLoad;
            TIM3->SR = 0;
            TIM3->CR1 = 1;

            while ((TIM3->SR & 0x1U) == 0)
                ;

            TIM3->CR1 = 0;
            break;
        case 10:  // 222Hz
            TIM4->ARR = 2252 + distortion;
            TIM4->CCR3 = (2252 / 4) * pwmLoad;
            TIM4->SR = 0;
            TIM4->CR1 = 1;

            while ((TIM4->SR & 0x1U) == 0)
                ;

            TIM4->CR1 = 0;
            break;
        case 11:  // 235Hz
            TIM3->ARR = 2128 + distortion;
            TIM3->CCR2 = (2128 / 4) * pwmLoad;
            TIM3->SR = 0;
            TIM3->CR1 = 1;

            while ((TIM3->SR & 0x1U) == 0)
                ;

            TIM3->CR1 = 0;
            break;
        case 12:  // 249Hz
            TIM4->ARR = 2008 + distortion;
            TIM4->CCR4 = (2008 / 4) * pwmLoad;
            TIM4->SR = 0;
            TIM4->CR1 = 1;

            while ((TIM4->SR & 0x1U) == 0)
                ;

            TIM4->CR1 = 0;
            break;
        case 13:  // 132Hz
            TIM2->ARR = 3788 + distortion;
            TIM2->CCR4 = (3788 / 4) * pwmLoad;
            TIM2->SR = 0;
            TIM2->CR1 = 1;

            while ((TIM2->SR & 0x1U) == 0)
                ;

            TIM2->CR1 = 0;
            break;
        case 14:  // 264Hz
            TIM2->ARR = 1894 + distortion;
            TIM2->CCR2 = (1894 / 4) * pwmLoad;
            TIM2->SR = 0;
            TIM2->CR1 = 1;

            while ((TIM2->SR & 0x1U) == 0)
                ;

            TIM2->CR1 = 0;
            break;
        case 15:  // 280Hz
            TIM2->ARR = 1786 + distortion;
            TIM2->CCR3 = (1786 / 4) * pwmLoad;
            TIM2->SR = 0;
            TIM2->CR1 = 1;

            while ((TIM2->SR & 0x1U) == 0)
                ;

            TIM2->CR1 = 0;
            break;
        case 16:  // 296Hz
            TIM3->ARR = 1690 + distortion;
            TIM3->CCR1 = (1690 / 4) * pwmLoad;
            TIM3->SR = 0;
            TIM3->CR1 = 1;

            while ((TIM3->SR & 0x1U) == 0)
                ;

            TIM3->CR1 = 0;
            break;
        case 17:  // 314Hz
            TIM3->ARR = 1592 + distortion;
            TIM3->CCR2 = (1592 / 4) * pwmLoad;
            TIM3->SR = 0;
            TIM3->CR1 = 1;

            while ((TIM3->SR & 0x1U) == 0)
                ;

            TIM3->CR1 = 0;
            break;
        case 18:  // 332Hz
            TIM2->ARR = 1506 + distortion;
            TIM2->CCR2 = (1506 / 4) * pwmLoad;
            TIM2->SR = 0;
            TIM2->CR1 = 1;

            while ((TIM2->SR & 0x1U) == 0)
                ;

            TIM2->CR1 = 0;
            break;
        case 19:  // 352Hz
            TIM2->ARR = 1420 + distortion;
            TIM2->CCR4 = (1420 / 4) * pwmLoad;
            TIM2->SR = 0;
            TIM2->CR1 = 1;

            while ((TIM2->SR & 0x1U) == 0)
                ;

            TIM2->CR1 = 0;
            break;
        case 20:  // 374Hz
            TIM3->ARR = 1336 + distortion;
            TIM3->CCR2 = (1336 / 4) * pwmLoad;
            TIM3->SR = 0;
            TIM3->CR1 = 1;

            while ((TIM3->SR & 0x1U) == 0)
                ;

            TIM3->CR1 = 0;
            break;
        case 21:  // 396Hz
            TIM2->ARR = 1264 + distortion;
            TIM2->CCR2 = (1264 / 4) * pwmLoad;
            TIM2->SR = 0;
            TIM2->CR1 = 1;

            while ((TIM2->SR & 0x1U) == 0)
                ;

            TIM2->CR1 = 0;
            break;
        case 22:  // 418Hz
            TIM3->ARR = 1196 + distortion;
            TIM3->CCR2 = (1196 / 4) * pwmLoad;
            TIM3->SR = 0;
            TIM3->CR1 = 1;

            while ((TIM3->SR & 0x1U) == 0)
                ;

            TIM3->CR1 = 0;
            break;
        case 23:  // 444Hz
            TIM4->ARR = 1126 + distortion;
            TIM4->CCR3 = (1126 / 4) * pwmLoad;
            TIM4->SR = 0;
            TIM4->CR1 = 1;

            while ((TIM4->SR & 0x1U) == 0)
                ;

            TIM4->CR1 = 0;
            break;
        case 24:  // 470Hz
            TIM3->ARR = 1064 + distortion;
            TIM3->CCR2 = (1064 / 4) * pwmLoad;
            TIM3->SR = 0;
            TIM3->CR1 = 1;

            while ((TIM3->SR & 0x1U) == 0)
                ;

            TIM3->CR1 = 0;
            break;
        case 25:  // 498Hz
            TIM4->ARR = 1004 + distortion;
            TIM4->CCR4 = (1004 / 4) * pwmLoad;
            TIM4->SR = 0;
            TIM4->CR1 = 1;

            while ((TIM4->SR & 0x1U) == 0)
                ;

            TIM4->CR1 = 0;
            break;
        case 26:  // 280Hz
            TIM2->ARR = 1786 + distortion;
            TIM2->CCR4 = (1786 / 4) * pwmLoad;
            TIM2->SR = 0;
            TIM2->CR1 = 1;

            while ((TIM2->SR & 0x1U) == 0)
                ;

            TIM2->CR1 = 0;
            break;
    }
}

void buzz(uint8_t note) {
    GPIOB->ODR ^= RPB_BUZZ;

    if (isFirstOctave) {
        engage_timer(note);
    } else {
        engage_timer(note + 13);
    }
}

void cycle_pwm_load() {
    if (debounceCounter > DEBOUNCE_TIMES) {
        switch (pwmLoad) {
            case 1:
                pwmLoad++;
                break;
            case 2:
                pwmLoad++;
                break;
            case 3:
                pwmLoad = 1;
                break;
        }
        debounceCounter = 0;
    } else {
        debounceCounter++;
    }
}