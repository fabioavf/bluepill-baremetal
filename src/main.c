#include <stdint.h>

#include "red_pill.h"
#include "stm32f1xx.h"

void lcd_init(void);
void lcd_command(unsigned char cmd);
void lcd_data(unsigned char data);
void lcd_print(char *str);
void lcd_putValue(unsigned char value);
void delay_us(uint16_t t);
void delay_ms(uint16_t t);

#define LED_DELAY 500000UL  // Ticks, 8MHz clock

int main(void) {
    uint32_t led_delay_count = 0;

    RCC->APB2ENR |= RCC_APB2ENR_IOPAEN;  // enable clock
    AFIO->MAPR |= AFIO_MAPR_SWJ_CFG_JTAGDISABLE;

    GPIOA->CRH = 0x34433443;
    GPIOA->CRL = 0x43344333;

    lcd_init();
    delay_ms(100);

    lcd_command(0x01);

    lcd_data(0x37);

    while (1) {
        GPIOA->ODR |= 0x00000007;  // led on
        delay_ms(50);

        GPIOA->ODR &= 0xFFFFFFF8;  // led off
        delay_ms(50);
    }

    /* Will never reach this return */
    return 0;
}

void lcd_init() {
    delay_ms(15);
    GPIOA->ODR &= ~RP_LCD_EN;

    delay_ms(3);
    lcd_command(0x33);

    delay_ms(5);
    lcd_command(0x32);

    delay_us(3000);
    lcd_command(0x28);

    delay_ms(3);
    lcd_command(0x0e);

    delay_ms(3);
    lcd_command(0x01);

    delay_ms(3);
    lcd_command(0x06);

    delay_ms(3);
}

void lcd_command(unsigned char cmd) {
    GPIOA->ODR &= ~RP_LCD_RS;
    lcd_putValue(cmd);
}

void lcd_data(unsigned char data) {
    GPIOA->ODR |= RP_LCD_RS;
    lcd_putValue(data);
}

void lcd_print(char *str) {
    unsigned char i = 0;

    while (str[i] != 0) {
        lcd_data(str[i]);
        i++;
    }
}

void lcd_putValue(unsigned char value) {
    uint16_t aux;                                            // variable to help to build appropriate data out
    aux = 0x0000;                                            // clear aux
    GPIOA->BRR = RP_LCD_4 | RP_LCD_5 | RP_LCD_6 | RP_LCD_7;  // clear data lines

    aux = value & 0xF0;
    aux = aux >> 4;

    GPIOA->BSRR = ((aux & 0x0008) << 8) | ((aux & 0x0004) << 3) | ((aux & 0x0002) << 5) | ((aux & 0x0001) << 8);
    GPIOA->ODR |= RP_LCD_EN;                                  // EN = 1 for H - to - L pulse
    delay_ms(3);                                              // make EN pulse wider
    GPIOA->ODR &= ~RP_LCD_EN;                                 // EN = 0 for H - to - L pulse
    delay_ms(1);                                              // wait
    GPIOA->BRR = (1 << 5) | (1 << 6) | (1 << 8) | (1 << 11);  // clear data lines
    aux = 0x0000;                                             // clear aux
    aux = value & 0x0F;
    GPIOA->BSRR = ((aux & 0x0008) << 8) | ((aux & 0x0004) << 3) | ((aux & 0x0002) << 5) | ((aux & 0x0001) << 8);
    GPIOA->ODR |= RP_LCD_EN;   // EN = 1 for H - to - L pulse
    delay_ms(3);               // make EN pulse wider
    GPIOA->ODR &= ~RP_LCD_EN;  // EN = 0 for H - to - L pulse
    delay_ms(1);
}

void delay_us(uint16_t t) {
    for (uint16_t i = 0; i < t; i++) {
        for (uint8_t j = 0; j < 6; j++) {
        }
    }
}

void delay_ms(uint16_t t) {
    for (uint16_t i = 0; i < t; i++) {
        for (uint16_t j = 0; j < 6000; j++) {
        }
    }
}
