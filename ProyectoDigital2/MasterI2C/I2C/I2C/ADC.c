/*
 * ADC.c
 *
 * Created: 26/1/2026 
 *  Author: santi
 */ 

#include "ADC.h"
//funcion inicializar el ADC
void init_ADC(uint8_t justificacion, uint8_t prescaler, uint8_t pin_adc){
	cli();
	ADMUX = 0; //borramos cualquier configuracion previa
	ADMUX |= (1<<REFS0);
	//coonfiguramos la justificacion
	switch(justificacion){
		case 0:
		ADMUX &= ~(1<<ADLAR); //Activamos la justificacion a la derecha
		break;
		case 1:
		ADMUX |= (1<<ADLAR);
		break;
		default:
		ADMUX |= (1<<ADLAR);
		break;
	}
	switch(pin_adc){
		case 0:
		ADMUX &= ~((1<<MUX0)|(1<<MUX1)|(1<<MUX2)|(1<<MUX3));
		break;
		case 1:
		ADMUX |= (1<<MUX0);
		break;
		case 2:
		ADMUX |= (1<<MUX1);
		break;
		case 3:
		ADMUX |= (1<<MUX0)|(1<<MUX1);
		break;
		case 4:
		ADMUX |= (1<<MUX2);
		break;
		case 5:
		ADMUX |= (1<<MUX2)|(1<<MUX0);
		break;
		case 6:
		ADMUX |= (1<<MUX2)|(1<<MUX1);
		break;
		case 7:
		ADMUX |= (1<<MUX2)|(1<<MUX1)|(1<<MUX0);
		break;
		default:
		ADMUX &= ~((1<<MUX0)|(1<<MUX1)|(1<<MUX2)|(1<<MUX3));
		break;
	}
	ADCSRA = 0; //borramos cualquier configuracion previa
	switch(prescaler){
		case 128:
		ADCSRA |= (1<<ADEN) | (1<<ADSC) | (1<<ADIE) | (1<<ADPS0) | (1<<ADPS1) | (1<<ADPS2);
		break;
		default:
		ADCSRA |= (1<<ADEN) | (1<<ADSC) | (1<<ADIE) | (1<<ADPS0) | (1<<ADPS1) | (1<<ADPS2);
		break;
	}
	sei();
}
void pinADC(uint8_t pin_adc, uint8_t just){
	ADMUX = 0;
	ADMUX |= (1<<REFS0);
	switch(just){
		case 0:
		ADMUX &= ~(1<<ADLAR); //Activamos la justificacion a la derecha
		break;
		case 1:
		ADMUX |= (1<<ADLAR);
		break;
		default:
		ADMUX |= (1<<ADLAR);
		break;
	}
	switch(pin_adc){
		case 0:
		ADMUX &= ~((1<<MUX0)|(1<<MUX1)|(1<<MUX2)|(1<<MUX3));
		break;
		case 1:
		ADMUX |= (1<<MUX0);
		break;
		case 2:
		ADMUX |= (1<<MUX1);
		break;
		case 3:
		ADMUX |= (1<<MUX0)|(1<<MUX1);
		break;
		case 4:
		ADMUX |= (1<<MUX2);
		break;
		case 5:
		ADMUX |= (1<<MUX2)|(1<<MUX0);
		break;
		case 6:
		ADMUX |= (1<<MUX2)|(1<<MUX1);
		break;
		case 7:
		ADMUX |= (1<<MUX2)|(1<<MUX1)|(1<<MUX0);
		break;
		default:
		ADMUX &= ~((1<<MUX0)|(1<<MUX1)|(1<<MUX2)|(1<<MUX3));
		break;
	}
}

