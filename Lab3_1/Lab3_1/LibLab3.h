/*
 * IncFile1.h
 *
 * Created: 29/1/2026 19:31:03
 *  Author: santi
 */ 


#ifndef LIBLAB3_H_
#define LIBLAB3_H_
#define F_CPU 16000000
#include <avr/io.h>
#include <avr/interrupt.h>
#include <stdint.h>
typedef enum{
	SPI_MASTER_OSC_DIV2 = 0b01010000,
	SPI_MASTER_OSC_DIV4 = 0b01010001,
	SPI_MASTER_OSC_DIV8 = 0b01010010,
	SPI_MASTER_OSC_DIV16 = 0b01010011,
	SPI_MASTER_OSC_DIV32 = 0b01010100,
	SPI_MASTER_OSC_DIV64 = 0b01010101,
	SPI_MASTER_OSC_DIV128 = 0b01010110,
	SPI_SLAVE_SS = 0b01000000
}SPI_TYPE;

typedef enum{
	SPI_DATA_ORDER_MSB = 0b00100000,
	SPI_DATA_ORDER_LSB = 0b00000000
}SPI_DATA_ORDER;

typedef enum{
	SPI_CLOCK_POLARITY_HIGH = 0b00001000,
	SPI_CLOCK_IDLE_LOW = 0b00000000
}SPI_CLOCK_POLARITY;

typedef enum{
	SPI_CLOCK_FIRST_EDGE = 0b00000000,
	SPI_CLOCK_LAST_EDGE = 0b00000100
}SPI_CLOCK_PHASE;
//protoripos de las funciones a utilizar
//prototipo para inicializar spi
void SPIinit(SPI_TYPE,SPI_DATA_ORDER, SPI_CLOCK_POLARITY, SPI_CLOCK_PHASE);
//prototipo para poder escribir
void SPIwrite(uint8_t dat);
unsigned SPIdataready();
uint8_t spiRead(void);

uint16_t LecturaADC(uint8_t PIN);

#endif /* LIBLAB3_H_ */