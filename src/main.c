#include <stdint.h>

#include "lcd.h"
#include "red_pill.h"
#include "stm32f10x.h"

int main(void) {
    RCC->APB2ENR = RCC_APB2ENR_IOPAEN | RCC_APB2ENR_IOPBEN | RCC_APB2ENR_AFIOEN;  // enable clock
    AFIO->MAPR |= AFIO_MAPR_SWJ_CFG_JTAGDISABLE;

    GPIOA->CRH = 0x34433443;
    GPIOA->CRL = 0x43344333;

    GPIOB->CRH = 0x44444444;
    GPIOB->CRL = 0x44444443;

    lcd_init();
    delay_ms(100);

    lcd_print("Hello world!");

    while (1) {
        if (!(GPIOB->IDR & RPB_SW1) || !(GPIOB->IDR & RPB_SW2)) {  // if sw1 pressed
            GPIOA->ODR |= (RPA_LED1 | RPA_LED2 | RPA_LED3);        // turn on leds

        } else {
            GPIOA->ODR &= ~(RPA_LED1 | RPA_LED2 | RPA_LED3);  // turn off leds
        }
    }

    /* Will never reach this return */
    return 0;
}