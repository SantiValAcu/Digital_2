/*
 * ADC.h
 *
 * Created: 26/1/2026 
 *  Author: santi
 */ 


#ifndef ADC_H_
#define ADC_H_
#include <avr/io.h>
#include <avr/interrupt.h>
#include <stdio.h>
//Prototipo de configuracion del ADC
void init_ADC(uint8_t justificacion, uint8_t prescaler, uint8_t pin_adc);
//funcion para seleccionar el pin del ADC
void pinADC(uint8_t pin_adc, uint8_t just);



#endif /* ADC_H_ */