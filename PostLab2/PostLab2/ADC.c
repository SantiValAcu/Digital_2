/*
 * ADC.c
 *
 * Created: 25/1/2026 
 *  Author: santi
 */ 

#include "adc.h"

/****************************************/
// Inicialización del ADC
void adc_init(uint8_t justificacion, uint8_t prescaler, uint8_t canal)
{
	// Referencia AVcc
	ADMUX = (1 << REFS0);

	// Justificación
	if (justificacion)
	ADMUX |= (1 << ADLAR);
	else
	ADMUX &= ~(1 << ADLAR);

	// Canal inicial
	ADMUX |= (canal & 0x07);

	// Habilitar ADC + interrupción
	ADCSRA = (1 << ADEN) | (1 << ADIE);

	// Prescaler
	switch (prescaler)
	{
		case 128:
		ADCSRA |= (1<<ADPS0) | (1<<ADPS1) | (1<<ADPS2);
		break;
		default:
		ADCSRA |= (1<<ADPS0) | (1<<ADPS1) | (1<<ADPS2);
		break;
	}

	// Iniciar primera conversión
	ADCSRA |= (1 << ADSC);
}

/****************************************/
// Selección de canal ADC
void adc_select_channel(uint8_t canal, uint8_t justificacion)
{
	ADMUX &= 0xF0;			// Limpia MUX
	ADMUX |= (1 << REFS0);	// AVcc
	ADMUX |= (canal & 0x07);

	if (justificacion)
	ADMUX |= (1 << ADLAR);
	else
	ADMUX &= ~(1 << ADLAR);
}