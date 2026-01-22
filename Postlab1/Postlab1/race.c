/*
 * race.c
 *
 * Created: 21/1/2026 08:22:33
 *  Author: santi
 */ 

#include <avr/io.h>
#include <avr/interrupt.h>
#include "race.h"
#include "display.h"
#include "player.h"

// Variable global que almacena el estado actual del sistema
extern volatile estado_t estado_actual;

static player_t jugador1;				// Estructura del jugador 1
static player_t jugador2;				// Estructura del jugador 2
static volatile uint8_t countdown;		// Valor del conteo regresivo
static volatile uint16_t ticks;			// Contador de overflows

void race_init(void)
{
	// Inicialización de los jugadores con su botón y puerto de LEDs
	player_init(&jugador1, (1 << PC4), &PORTB);
	player_init(&jugador2, (1 << PC5), &PORTC);
}

/****************************************/
// Logica principal
void race_update(void)
{
	uint8_t inicio = !(PIND & (1 << PD7));	// Botón de inicio

	switch (estado_actual)
	{
		case ESTADO_IDLE:
		// Estado de reposo
		display_show(0);
		player_reset(&jugador1);
		player_reset(&jugador2);

		if (inicio)
		{
			countdown = 5;
			display_show(5);
			estado_actual = ESTADO_COUNTDOWN;
		}
		break;

		case ESTADO_COUNTDOWN:
		// Durante el conteo los LEDs permanecen apagados
		PORTB &= ~0x0F;			// LEDs jugador 1
		PORTC &= ~0x0F;			// LEDs jugador 2
		
		// Permite reiniciar el conteo
		if (inicio)
		{
			countdown = 5;
			display_show(5);
		}
		break;

		case ESTADO_CARRERA:
		// Permite reiniciar la carrera desde este estado
		if (inicio)
		{
			countdown = 5;
			display_show(5);
			estado_actual = ESTADO_COUNTDOWN;
			break;
		}
		// Actualización de los jugadores
		player_update(&jugador1, &PINC);
		player_update(&jugador2, &PINC);
		// Detección de ganador
		if (player_has_won(&jugador1))
		{
			PORTB = 0x0F;		// LEDs jugador 1 encendidos
			PORTC = 0x00;		// LEDs jugador 2 apagados
			display_show(1);	// Mostrar numero 1 en display
			estado_actual = ESTADO_GANADOR;
		}
		else if (player_has_won(&jugador2))
		{
			PORTC = 0x0F;		// LEDs jugador 2 encendidos
			PORTB = 0x00;		// LEDs jugador 1 apagados
			display_show(2);	// Msotrar numero 2 en display
			estado_actual = ESTADO_GANADOR;
		}
		break;

		case ESTADO_GANADOR:
		// Espera a que se presione inicio para reiniciar el sistema
		if (inicio)
		estado_actual = ESTADO_IDLE;
		break;
	}
}

/****************************************/
// ISR Timer0 Overflow (~16.38 ms)
ISR(TIMER0_OVF_vect)
{
	if (estado_actual == ESTADO_COUNTDOWN)
	{
		ticks++;
		if (ticks >= 61)			// 61 overflows ? 1 segundo
		{
			ticks = 0;
			if (countdown > 0)
			{
				countdown--;
				display_show(countdown);
			}
			else
			{
				// Finaliza el conteo e inicia la carrera
				player_reset(&jugador1);
				player_reset(&jugador2);
				estado_actual = ESTADO_CARRERA;
			}
		}
	}
}