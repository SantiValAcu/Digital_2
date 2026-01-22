/*
 * player.h
 *
 * Created: 21/1/2026
 *  Author: santi
 */ 


#ifndef PLAYER_H_
#define PLAYER_H_

#include <stdint.h>

// Estructura para los jugadores "player_t"
typedef struct {
	uint8_t value;						// Valor actual del contador (0 a 4)
	uint8_t button_mask;				// Máscara del pin asociado al botón del jugador
	volatile uint8_t *led_port;			// Puntero al puerto donde están conectados los LEDs
	uint8_t last_state;					// Estado previo del botón
} player_t;

// Funcion para asignar el puerto de LEDs, el botón correspondiente y deja el contador y estado inicial en cero
void player_init(player_t *p, uint8_t mask, volatile uint8_t *port);
// Funcion para reinicia el estado del jugador. Contador en cero y leds apagados
void player_reset(player_t *p);
// Funcion para detecta un flanco del botón y, si ocurre, incrementa el contador y actualiza los LEDs
void player_update(player_t *p, volatile uint8_t *pin_reg);
// Verificacion si el jugador ha alcanzado la meta (4 leds encendidos)
uint8_t player_has_won(player_t *p);

#endif /* PLAYER_H_ */