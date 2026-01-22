/*
 * display.c
 *
 * Created: 21/1/2026
 *  Author: santi
 */ 

#include <avr/io.h>
#include "display.h"
// Carpeta de numeros en display 0, 1, 2, 3, 4, 5
static const uint8_t display_table[] = {
	0x77, 0x42, 0x6D, 0x6B, 0x5A, 0x3B
};

// Funcion para mostrar los numeros en display
void display_show(uint8_t num)
{
	if (num <= 5)				//si variable num es menor o igual mostrar e display
	PORTD = display_table[num];
}