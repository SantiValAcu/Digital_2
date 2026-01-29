/*
 * LCD.c
 *
 * Created: 25/1/2026 16:52:00
 *  Author: santi
 */ 

#define F_CPU 16000000UL
#include "lcd.h"
#include <util/delay.h>

/****************************************/
// Función interna para enviar datos al bus
static void lcd_port(char data);
/****************************************/

// Inicialización del LCD
void lcd_init(void)
{
	// Puerto D ? D2–D7 (datos)
	DDRD |= (1<<DDD2)|(1<<DDD3)|(1<<DDD4)|(1<<DDD5)|(1<<DDD6)|(1<<DDD7);
	PORTD &= ~((1<<PORTD2)|(1<<PORTD3)|(1<<PORTD4)|(1<<PORTD5)|(1<<PORTD6)|(1<<PORTD7));

	// Puerto B ? B0–B1 (datos)
	DDRB |= (1<<DDB0)|(1<<DDB1);
	PORTB &= ~((1<<PORTB0)|(1<<PORTB1));

	// Puerto C ? RS (PC0), RW (PC1), EN (PC2)
	DDRC |= (1<<DDC0)|(1<<DDC1)|(1<<DDC2);
	PORTC &= ~((1<<PORTC0)|(1<<PORTC1)|(1<<PORTC2));

	lcd_port(0x00);
	_delay_ms(20);

	lcd_cmd(0x30);
	_delay_ms(5);
	lcd_cmd(0x30);
	_delay_ms(5);
	lcd_cmd(0x30);
	_delay_ms(1);

	lcd_cmd(0x38);	// 8 bits, 2 líneas
	lcd_cmd(0x08);	// Display OFF
	lcd_cmd(0x01);	// Clear
	_delay_ms(3);
	lcd_cmd(0x06);	// Incremento cursor
	lcd_cmd(0x0C);	// Display ON
}

// Enviar comando
void lcd_cmd(char cmd)
{
	PORTC &= ~(1<<PORTC0);	// RS = 0
	lcd_port(cmd);
	PORTC |= (1<<PORTC2);
	_delay_ms(4);
	PORTC &= ~(1<<PORTC2);
}

// Enviar carácter
void lcd_write_char(char data)
{
	PORTC |= (1<<PORTC0);	// RS = 1
	lcd_port(data);
	PORTC |= (1<<PORTC2);
	_delay_ms(4);
	PORTC &= ~(1<<PORTC2);
}

// Enviar cadena
void lcd_write_string(char *str)
{
	while (*str)
	{
		lcd_write_char(*str++);
	}
}

// Posicionar cursor
void lcd_set_cursor(char col, char row)
{
	char address;

	if (row == 1)
	address = 0x80 + col - 1;
	else
	address = 0xC0 + col - 1;

	lcd_cmd(address);
}

// Desplazamientos
void lcd_shift_right(void)
{
	lcd_cmd(0x1C);
}

void lcd_shift_left(void)
{
	lcd_cmd(0x18);
}

/****************************************/
// Función privada
static void lcd_port(char data)
{
	(data & (1<<0)) ? (PORTD |= (1<<PORTD2)) : (PORTD &= ~(1<<PORTD2));
	(data & (1<<1)) ? (PORTD |= (1<<PORTD3)) : (PORTD &= ~(1<<PORTD3));
	(data & (1<<2)) ? (PORTD |= (1<<PORTD4)) : (PORTD &= ~(1<<PORTD4));
	(data & (1<<3)) ? (PORTD |= (1<<PORTD5)) : (PORTD &= ~(1<<PORTD5));
	(data & (1<<4)) ? (PORTD |= (1<<PORTD6)) : (PORTD &= ~(1<<PORTD6));
	(data & (1<<5)) ? (PORTD |= (1<<PORTD7)) : (PORTD &= ~(1<<PORTD7));
	(data & (1<<6)) ? (PORTB |= (1<<PORTB0)) : (PORTB &= ~(1<<PORTB0));
	(data & (1<<7)) ? (PORTB |= (1<<PORTB1)) : (PORTB &= ~(1<<PORTB1));
}