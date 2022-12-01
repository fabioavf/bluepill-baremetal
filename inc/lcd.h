#ifndef LCD_H
#define LCD_H

#include "red_pill.h"
#include "stdint.h"
#include "stm32f10x.h"

void lcd_init(void);
void lcd_command(unsigned char cmd);
void lcd_data(unsigned char data);
void lcd_print(char *str);
void lcd_putValue(unsigned char value);
void delay_us(uint16_t t);
void delay_ms(uint16_t t);

#endif