/*
 * race.h
 *
 * Created: 21/1/2026 08:22:50
 *  Author: santi
 */ 


#ifndef RACE_H_
#define RACE_H_

// Estructura de enumeracion de los estados del sistema "estado_t"
typedef enum {
	ESTADO_IDLE,			// Sistema en reposo, esperando inicio de carrera 
	ESTADO_COUNTDOWN,		// Conteo regresivo previo a la carrera
	ESTADO_CARRERA,			// Los jugadores pueden presionar sus botones
	ESTADO_GANADOR			// Se muestra el ganador y se bloquean entradas
} estado_t;

// Funcion para inicializar los elementos necesarios para la carrera
void race_init(void);
//Funcion principal de la maquina de estados
void race_update(void);

#endif /* RACE_H_ */