/*
 * Postlab1.c
 *
 * Author: Santiago Valdez Acuña
 * Description: Carrera de dos jugadores con conteo regresivo y detección de ganador
 */
/****************************************/
// Encabezado (Libraries)
#include <avr/io.h>
#include <avr/interrupt.h>
#include "display.h"			// Libreria de enumeracion del display
#include "player.h"				// Libreria de logica de jugadores
#include "race.h"				// Libreria de logica de juego
// Nombres variables
volatile estado_t estado_actual = ESTADO_IDLE;
/****************************************/
// Function prototypes
void setup(void);
/****************************************/
// Main Function
int main(void)
{
	setup();				// configurar MCU

	while (1)
	{
		race_update();		// Maneja la lógica por estados
	}
}
/****************************************/
// NON-Interrupt subroutines
void setup(void)
{
	cli();					// Desactivar interrupciones globales

	// Configuracion puerto D
	DDRD = 0x7F;                 // PD0–PD6 salida (display)
	DDRD &= ~(1 << PD7);         // PD7 entrada
	PORTD |= (1 << PD7);         // Pull-up botón inicio

	// Configuracion puerto B 
	DDRB |= 0x0F;				// PB0-PD3 salida (leds jugador 1)
	PORTB &= ~0x0F;				// Leds apagados

	// Configuracion puerto C
	DDRC |= 0x0F;				// PC0-PC3 salida (leds jugador 2)
	PORTC &= ~0x0F;				// Leds apagados

	DDRC &= ~((1 << PC4) | (1 << PC5));		// PC4 y PC5 como entrada
	PORTC |= (1 << PC4) | (1 << PC5);		// Pull-ups botones de jugadores

	// Timer0 – modo normal
	TCCR0A = 0x00;
	TCCR0B = (1 << CS02) | (1 << CS00);		// Prescaler 1024
	TIMSK0 = (1 << TOIE0);					// Habilitar overflow

	// Display inicia en 0
	display_show(0);
	
	race_init();

	sei();						// Habilitar interrupciones globales
}
/****************************************/