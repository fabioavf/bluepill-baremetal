#include "lcd.h"

void lcd_init(void) {
    delay_ms(15);
    GPIOA->ODR &= ~RPA_LCD_EN;

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
    GPIOA->ODR &= ~RPA_LCD_RS;
    lcd_putValue(cmd);
}

void lcd_data(unsigned char data) {
    GPIOA->ODR |= RPA_LCD_RS;
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
    uint16_t aux = 0x0000;

    GPIOA->BRR = RPA_LCD_4 | RPA_LCD_5 | RPA_LCD_6 | RPA_LCD_7;
    aux = value & 0xF0;
    aux = aux >> 4;

    GPIOA->BSRR = ((aux & 0x0008) << 8) | ((aux & 0x0004) << 3) | ((aux & 0x0002) << 5) | ((aux & 0x0001) << 8);

    GPIOA->ODR |= RPA_LCD_EN;
    delay_ms(3);
    GPIOA->ODR &= ~RPA_LCD_EN;
    delay_ms(1);

    GPIOA->BRR = RPA_LCD_4 | RPA_LCD_5 | RPA_LCD_6 | RPA_LCD_7;

    aux = 0x0000;
    aux = value & 0x0f;

    GPIOA->BSRR = ((aux & 0x0008) << 8) | ((aux & 0x0004) << 3) | ((aux & 0x0002) << 5) | ((aux & 0x0001) << 8);
    GPIOA->ODR |= RPA_LCD_EN;
    delay_ms(3);

    GPIOA->ODR &= ~RPA_LCD_EN;
    delay_ms(1);
}

void delay_us(uint16_t t) {
    for (uint16_t i = 0; i < t; i++)
        for (volatile uint32_t l = 0; l < 6; l++)
            ;
}

void delay_ms(uint16_t t) {
    for (uint16_t i = 0; i < t; i++) {
        for (volatile uint32_t l = 0; l < 6000; l++)
            ;
    }
}