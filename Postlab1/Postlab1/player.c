/*
 * player.c
 *
 * Created: 21/1/2026 08:21:17
 *  Author: santi
 */ 

#include "player.h"

// Inicializacion de los parametros del jugador
void player_init(player_t *p, uint8_t mask, volatile uint8_t *port)
{
	p->value = 0;				// COntador en cero == leds apagados
	p->button_mask = mask;
	p->led_port = port;
	p->last_state = 1;			// Boton pull-up
}

// Reinicio de valores para una nueva carrera
void player_reset(player_t *p)
{
	p->value = 0;				// Contador en cero == leds apagados
	*(p->led_port) &= ~0x0F;	// Leds apagados
	p->last_state = 1;			// Boton pull-up
}

// Actualizacion del estado del jugador durante la carrera
void player_update(player_t *p, volatile uint8_t *pin_reg)
{
	uint8_t current = (*pin_reg & p->button_mask);

	// Deteccion de flanco de bajada
	if (!current && p->last_state)
	{
		// Incremento del contador hasta el maximo. "4"
		if (p->value < 4)
		p->value++;
		// Actualización de LEDs según el valor del contador
		if (p->value < 4)
		*(p->led_port) = (1 << (p->value - 1)); // 0001-0010-0100
		else
		*(p->led_port) = 0x0F;                  // ganador 1111
	}
	// Guardar estado actual del botón
	p->last_state = current;
}

// Verificacion si el jugador ha llegado a la meta
uint8_t player_has_won(player_t *p)
{
	return (p->value >= 4);
}