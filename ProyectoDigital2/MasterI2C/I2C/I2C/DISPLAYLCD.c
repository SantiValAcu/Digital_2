/*
 * DISPLAYLCD.c
 *
 * Created: 26/1/2026
 *  Author: santi
 */ 
#include "DISPLAYLCD.h"
//Funcion para inicializar el LCD
void init8bits(void){
	//configuramos los puertos que vamos a utilizar como salida
	//configuramos el puerto D
	DDRD |= (1<<DDD2)|(1<<DDD3)|(1<<DDD4)|(1<<DDD5)|(1<<DDD6)|(1<<DDD7);
	//apamos todos esos pines
	PORTD &= ~((1<<PORTD2)|(1<<PORTD3)|(1<<PORTD4)|(1<<PORTD5)|(1<<PORTD6)|(1<<PORTD7));
	//Configuracion de pines en el puerto B
	DDRB |= (1<<DDB0) | (1<<DDB1);
	//apagamos dichos puertos
	PORTB &= ~((1<<PORTB0) | (1<<PORTB1));
	//Configuramos el puerto C
	DDRC |= (1<<DDC0) | (1<<DDC1) | (1<<DDC2);
	//Apagamos esos pines
	PORTC &= ~((1<<PORTC0) | (1<<PORTC1) | (1<<PORTC2));
	//inicializamos todos los bits en 0
	LCDPORT(0X00);
	_delay_ms(20); //Hacemos pausa de 20 ms que le toma al display funcionar
	LCDCMD(0x30);
	_delay_ms(5);
	LCDCMD(0x30);
	_delay_ms(5);
	LCDCMD(0X30);
	_delay_ms(1);
	//FUNCTION SET
	LCDCMD(0x38);
	_delay_ms(1);
	LCDCMD(0x08);
	_delay_ms(1);
	LCDCMD(0x01);
	_delay_ms(3);
	LCDCMD(0x06);
	_delay_ms(1);
	LCDCMD(0x0C);
	_delay_ms(1);
}
void LCDCMD(char a){
	//ponemos en RS en 0 para hacer que lo reconosca como un comando
	PORTC &= ~(1<<PORTC0);
	//mandamos al puerto
	LCDPORT(a);
	//Activamos EN
	PORTC |= (1<<PORTC2);
	_delay_ms(5);
	//Desactivamos EN
	PORTC &= ~(1<<PORTC2);
}
void LCDPORT(char a){
	//Analizamos si el primer bit esta en 0 o 1
	if((a >> 0) & 1) PORTD |= (1<<PORTD2);
	else PORTD &= ~(1<<PORTD2);
	//Analizamos si el segundo bit esta en 0 o 1
	if((a >> 1) & 1) PORTD |= (1<<PORTD3);
	else PORTD &= ~(1<<PORTD3);
	//Analizamos si el tercer bit esta en 0 o 1
	if((a >> 2) & 1) PORTD |= (1<<PORTD4);
	else PORTD &= ~(1<<PORTD4);
	//Analizamos si el cuarto bit esta en 0 o 1
	if((a >> 3) & 1) PORTD |= (1<<PORTD5);
	else PORTD &= ~(1<<PORTD5);
	//Analizamos si el quinto bit esta en 0 o 1
	if((a >> 4) & 1) PORTD |= (1<<PORTD6);
	else PORTD &= ~(1<<PORTD6);
	//Analizamos si el sexto bit esta en 0 o 1
	if((a >> 5) & 1) PORTD |= (1<<PORTD7);
	else PORTD &= ~(1<<PORTD7);
	//Analizamos si el septimo bit esta en 0 o 1
	if((a >> 6) & 1) PORTB |= (1<<PORTB0);
	else PORTB &= ~(1<<PORTB0);
	//Analizamos si el octavo bit esta en 0 o 1
	if((a >> 7) & 1) PORTB |= (1<<PORTB1);
	else PORTB &= ~(1<<PORTB1);
}
void LCD_WRITE_CHAR(char c){
	//Ponemos RS en 1 para poder mandar un dato
	PORTC |= (1<<PORTC0);
	LCDPORT(c);
	//Hacemos la transicion del enable
	PORTC |= (1<<PORTC2);
	_delay_ms(5);
	PORTC &= ~(1<<PORTC2);
}
//Funcion para enviar una cadena
void LCD_WRITE_STRING(char *a){
	int i;
	for(i=0; a[i] != '\0'; i++)
	LCD_WRITE_CHAR(a[i]);
}
//establecer cursor del caracter
void LCD_SET_CURSOR(char c, char f){
	char temp;
	if(f == 1){
		temp = 0x80 + c - 1;
		LCDCMD(temp);
	}
	else if(f == 2){
		temp = 0xC0 + c - 1;
		LCDCMD(temp);
	}
}
//desplazamiento a la derecha
void LCD_SHIFT_RIGHT(void){
	LCDCMD(0X1C);
}
//desplazamiento a la izquierda
void LCD_SHIFT_LEFT(void){
	LCDCMD(0X18);
}