/*
 * PostLab2.c
 *
 * Created: 25/1/2026 16:42:46
 * Author: Santiago Valdez Acuña
 * Description: Lectura ADC, LCD 16x2 y contador por UART
 */
 /****************************************/

// Encabezado (Libraries)
#define F_CPU 16000000UL
#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/delay.h>
#include <stdlib.h>

// Librerías del proyecto
#include "LCD.h"
#include "ADC.h"
#include "UART.h"
/****************************************/
// Variables globales
volatile uint8_t MULTIPLEXADO = 0;
volatile uint16_t POT1;
volatile uint16_t POT2;

float ADC1;
char t[16];
char buf[10];

volatile uint8_t contador = 0;
volatile char dato;
/****************************************/
// Function prototypes
void setup(void);
/****************************************/
// Main Function
int main(void)
{
	setup();

	while (1)
	{
		// Todo se maneja por interrupciones
	}
}
/****************************************/
// NON-Interrupt subroutines
void setup(void)
{
	cli();

	lcd_init();
	UART_RECEIVER(103);

	// ADC: justificación derecha, prescaler 128, canal ADC3
	adc_init(0, 128, 3);

	cadena("Contador. Envie un '+' o '-'...\n");

	sei();
}
/****************************************/
// Interrupt routines
ISR(ADC_vect)
{
	switch (MULTIPLEXADO)
	{
		case 1:
		POT1 = ADC;
		ADC1 = (POT1 * 5.0) / 1023.0;
		dtostrf(ADC1, 4, 2, t);

		lcd_set_cursor(2,1);
		lcd_write_string("S1:");

		lcd_set_cursor(1,2);
		lcd_write_string(t);
		lcd_write_string("V");

		adc_select_channel(4, 0);
		break;

		case 2:
		POT2 = ADC;
		itoa(POT2, buf, 10);

		lcd_set_cursor(8,1);
		lcd_write_string("S2:");

		lcd_set_cursor(7,2);
		lcd_write_string(buf);
		lcd_write_string("  ");
		break;

		default:
		MULTIPLEXADO = 0;
		adc_select_channel(3, 0);
		break;
	}

	if (contador == 0)
	{
		lcd_set_cursor(13,1);
		lcd_write_string("S3:");
		lcd_set_cursor(13,2);
		lcd_write_string("0");
	}

	MULTIPLEXADO++;
	ADCSRA |= (1 << ADSC);
}

ISR(USART_RX_vect)
{
	dato = UDR0;

	if (dato == '+') contador++;
	else if (dato == '-') contador--;
	else cadena("NO VALIDO\n");

	itoa(contador, buf, 10);

	lcd_set_cursor(13,1);
	lcd_write_string("S3");
	lcd_set_cursor(13,2);
	lcd_write_string(buf);
	lcd_write_string("   ");

	cadena("Contador S3, Envie un '+' o '-'...\n");
}
/****************************************/