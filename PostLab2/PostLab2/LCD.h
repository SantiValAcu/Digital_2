/*
 * LCD.h
 *
 * Created: 25/1/2026 
 *  Author: santi
 */ 


#ifndef LCD_H_
#define LCD_H_

#include <avr/io.h>
#include <stdint.h>

// Inicialización del LCD en modo 8 bits
void lcd_init(void);

// Envío de comandos y datos
void lcd_cmd(char cmd);
void lcd_write_char(char data);
void lcd_write_string(char *str);

// Control de cursor y desplazamiento
void lcd_set_cursor(char col, char row);
void lcd_shift_right(void);
void lcd_shift_left(void);

#endif /* LCD_H_ */