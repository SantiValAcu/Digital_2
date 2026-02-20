/*
 * DISPLAYLCD.h
 *
 * Created: 26/1/2026
 *  Author: santi
 */ 


//libreria creada a partir de la libreria para inicializar con 4 bits
#ifndef DISPLAYLCD_H_
#define DISPLAYLCD_H_
#define F_CPU 16000000
#include <avr/io.h>
#include <util/delay.h>
//prototipo de inicializacion de display de 8 bits
void init8bits(void);
//funcion para enviar
void LCDPORT(char a);
//funcion para enviar un comando
void LCDCMD(char a);
//Funcion para escribir un caracter
void LCD_WRITE_CHAR(char c);
//Funcion para enviar una cadena
void LCD_WRITE_STRING(char *a);
//Desplazamiento a la derecha
void LCD_SHIFT_RIGHT(void);
//Desplazamiento a la izquierda
void LCD_SHIFT_LEFT(void);
//establecer el cursor
void LCD_SET_CURSOR(char c, char f);

#endif /* DISPLAYLCD_H_ */