/*
 * Lab3_1.c
 *
 * Created: 29/1/2026	
 * Author : santi
 */ 

#define F_CPU 16000000
#include <avr/io.h>
#include <avr/interrupt.h>
#include <stdint.h>
#include "LibLAB3.h"
volatile uint8_t spi_cmd = 0;
uint8_t valorSPI = 0;
/****************************************/
// Function prototypes
void refreshPORT(uint8_t valor);
void setup(void);
/****************************************/
// Main Function
int main(void)
{
    setup();
	DDRD |= 0b11111100;  // D2–D7
	DDRB |= 0b00000011;  // D8–D9
    SPIinit(SPI_SLAVE_SS, SPI_DATA_ORDER_MSB,
            SPI_CLOCK_IDLE_LOW, SPI_CLOCK_FIRST_EDGE);

    SPDR = 0x00;   // primer dummy obligatorio

    while (1)
    {
        if (SPSR & (1 << SPIF))
        {
            uint8_t rx = SPDR;   // byte recibido

            // Si llega comando
            if (rx == 'c' || rx == 'd') {
                spi_cmd = rx;
                SPDR = 0x00;     // dummy para siguiente clock
            }
            else {
                // Este byte es el dummy del master ? RESPONDER
                if (spi_cmd == 'c') {
                    SPDR = (uint8_t)(LecturaADC(2) >> 2);
                }
                else if (spi_cmd == 'd') {
                    SPDR = (uint8_t)(LecturaADC(3) >> 2);
                }
                else {
                    SPDR = 0x00;
                }
            }
        }
		
uint8_t adc8 = LecturaADC(2) >> 2;

// D2–D7
PORTD = (PORTD & 0b00000011) | (adc8 << 2);

// D8–D9
PORTB = (PORTB & 0b11111100) | (adc8 >> 6);
    }
}