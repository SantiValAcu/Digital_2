/*
 * LibLab3.c
 *
 * Created: 29/1/2026 19:31:35
 *  Author: santi
 */ 


#include "LibLAB3.h"

void refreshPORT(uint8_t valor){
	if(valor & 0b10000000){
		PORTB |= (1<<PORTB1);
	}
	else{
		PORTB &= ~(1<<PORTB1);
	}
	if(valor & 0b01000000){
		PORTB |= (1<<PORTB0);
	}
	else{
		PORTB &= ~(1<<PORTB0);
	}
	if(valor & 0b00100000){
		PORTD |= (1<<PORTD7);
	}
	else{
		PORTD &= ~(1<<PORTD7);
	}
	if(valor & 0b00010000){
		PORTD |= (1<<PORTD6);
	}
	else{
		PORTD &= ~(1<<PORTD6);
	}
	if(valor & 0b00001000){
		PORTD |= (1<<PORTD5);
	}
	else{
		PORTD &= ~(1<<PORTD5);
	}
	if(valor & 0b00000100){
		PORTD |= (1<<PORTD4);
	}
	else{
		PORTD &= ~(1<<PORTD4);
	}
	if(valor & 0b00000010){
		PORTD |= (1<<PORTD3);
	}
	else{
		PORTD &= ~(1<<PORTD3);
	}
	if(valor & 0b00000001){
		PORTD |= (1<<PORTD2);
	}
	else{
		PORTD &= ~(1<<PORTD2);
	}
	
}
void setup(){
    // --- ADC ---
    ADMUX = (1 << REFS0);
    ADCSRA = (1 << ADEN) | (1 << ADPS2) | (1 << ADPS1) | (1 << ADPS0);

    // --- SPI SLAVE ---
    DDRB &= ~(1 << DDB2);    // SS (PB2) como ENTRADA
    PORTB |= (1 << PORTB2); // pull-up en SS

    DDRB |= (1 << DDB4);    // MISO como SALIDA
    DDRB &= ~((1 << DDB3) | (1 << DDB5)); // MOSI, SCK como ENTRADA

    // --- LEDs (igual que antes) ---
    DDRD |= (1<<DDD2)|(1<<DDD3)|(1<<DDD4)|(1<<DDD5)|(1<<DDD6)|(1<<DDD7);
    DDRB |= (1<<DDB0)|(1<<DDB1);

    PORTB &= ~((1<<PORTB0)|(1<<PORTB1));
    PORTD &= ~((1<<DDD2)|(1<<DDD3)|(1<<DDD4)|(1<<DDD5)|(1<<DDD6)|(1<<DDD7));
}

void SPIinit(SPI_TYPE stype, SPI_DATA_ORDER	sDataOrder, SPI_CLOCK_POLARITY sClockPolarity, SPI_CLOCK_PHASE sClockPhase){
	//PB2 ---> SS
	//PB3 ---> MOSI
	//PB4 ---> MISO
	//PB5 ---> SCK
	
	if(stype & (1<<MSTR)){
		DDRB |= (1<<DDB3)|(1<<DDB5)|(1<<DDB2); //MOSI. SCK, NEGADO_SS
		DDRB &= ~(1<<DDB4);
		SPCR |= (1<<MSTR);
		
		uint8_t temp = stype & 0b00000111;
		switch(temp){
			case 0: //DIV2
			SPCR &= ~((1<<SPR1)|(1<<SPR0));
			SPSR |= (1<<SPI2X);
			break;
			case 1: //DIV 4
			SPCR &= ~((1<<SPR1)|(1<<SPR0));
			SPSR &= ~(1<<SPI2X);
			break;
			case 2: //DIV8
			SPCR |= (1<<SPR0);
			SPCR &= ~(1<<SPR1);
			SPSR |= (1<<SPI2X);
			case 3: //DIV16
			SPCR |= (1<<SPR0);
			SPCR &= ~(1<<SPR1);
			SPSR &= ~(1<<SPI2X);
			break;
			case 4: //DIV32
			SPCR &= ~(1<<SPR0);
			SPCR |= (1<<SPR1);
			SPSR |= (1<<SPI2X);
			break;
			case 5: //DIV64
			SPCR &= ~(1<<SPR0);
			SPCR |= (1<<SPR1);
			SPSR &= ~(1<<SPI2X);
			break;
			case 6: //DIV128
			SPCR |= (1<<SPR0)|(1<<SPR1);
			SPSR &= ~(1<<SPI2X);
			break;
		}
	}
	else { // SLAVE
    DDRB |= (1<<DDB4); // MISO salida
    DDRB &= ~((1<<DDB3)|(1<<DDB5)|(1<<DDB2)); // MOSI, SCK, SS entrada
    PORTB |= (1<<PORTB2); // ?? pull-up en SS
    SPCR &= ~(1<<MSTR);
}
	//ENABLE SPI, DATA ORDER, CLOCK POLARITY, CLOCK PHASE
	SPCR |= (1<<SPE)|sDataOrder|sClockPolarity|sClockPhase;
}
static void spiReceivewait(){ // Espera a que la informacion sea recibida por completo
	while (!(SPSR & (1<<SPIF)));
}

void SPIwrite(uint8_t dat){ //escribir datos al bus de SPI
	SPDR = dat;
}
unsigned SPIdataready(){ //ver si la info esta lista para ser leida
	if(SPSR & (1<<SPIF))
	return 1;
	else
	return 0;
}
uint8_t spiRead(void){ //lee la informacion recibida
	while(!(SPSR & (1<<SPIF))); //esperar a la informacion este completa
	return(SPDR); //leer la informacion recibida desde el buffer
}

uint16_t LecturaADC(uint8_t PIN){ //Cambiar a uint16_t cuando se vaya a usar servos
	ADMUX = (ADMUX & 0xF0) | PIN; // Seleccionar canal
	ADCSRA |= (1 << ADSC); // Iniciar conversión
	while (ADCSRA & (1 << ADSC)); // Esperar a que termine
	return ADC; //cambiar a solo ADC cuando se vaya a usar servos
	
}