	/*
 * UART.c
 *
 * Created: 29/1/2026 17:18:02
 *  Author: santi
 */ 

#include "UART.h"

void UART_RECEIVER(uint8_t baudrate){
	// TX salida, RX entrada
	DDRD |= (1<<DDD1);   // TX
	DDRD &= ~(1<<DDD0);  // RX
	
	UCSR0A = 0;
	UCSR0B = (1 << RXCIE0) | (1 << RXEN0) | (1 << TXEN0);
	UCSR0C = (1 << UCSZ01) | (1 << UCSZ00);
	UBRR0 = baudrate; // 9600 a 16MHz
}

// Enviar un caracter
void writechar(char caracter){
	while((UCSR0A & (1 << UDRE0)) == 0);
	UDR0 = caracter;
}

// Enviar una cadena
void cadena(char* frase){
	for(uint8_t i = 0; *(frase+i) != '\0'; i++){
		writechar(*(frase+i));
	}
}