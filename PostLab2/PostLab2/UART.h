/*
 * UART.h
 *
 * Created: 29/1/2026 17:17:48
 *  Author: santi
 */ 


#ifndef UART_H_
#define UART_H_

#include <avr/io.h>
#include <avr/interrupt.h>
#include <stdio.h>

// funcion para poder comunicarme con el nano
void UART_RECEIVER(uint8_t baudrate);
void writechar(char caracter);
void cadena(char* frase);

#endif /* UART_H_ */