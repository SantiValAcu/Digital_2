/*
 * ADC.h
 *
 * Created: 25/1/2026
 *  Author: santi
 */ 


#ifndef ADC_H_
#define ADC_H_

#include <avr/io.h>
#include <stdint.h>
#include <avr/interrupt.h>

// Inicialización del ADC
void adc_init(uint8_t justificacion, uint8_t prescaler, uint8_t canal);

// Selección de canal ADC
void adc_select_channel(uint8_t canal, uint8_t justificacion);

#endif /* ADC_H_ */