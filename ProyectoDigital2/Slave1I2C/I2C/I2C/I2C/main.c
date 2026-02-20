/*
 * I2C.c
 *
 * Created: 5/2/2026 10:47:43
 * Author : Rodri
 */ 

//ESTE ES EL ESCLAVO
//ESCLAVO
//SLAVE
#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/delay.h>
#include "I2C.h"

#define  SlaverAddress 0x30
//#define  SlaverAddress 0x40

uint8_t buffer = 0;
volatile uint8_t valorADC =0;
uint8_t Foto1 = 0;
uint8_t Foto2 = 0;
uint8_t Foto3 = 0;
uint16_t Prom = 0;
uint8_t DiaNoche = 0;

void initADC(void);

int main(void)
{
	DDRD |= (1<<DDD2);
	PORTD &= ~(1<<PORTD2);
	
	DDRD |= (1<<DDD2)|(1<<DDD3)|(1<<DDD4)|(1<<DDD5)|(1<<DDD6)|(1<<DDD7);
	DDRB |= (1<<DDB0)|(1<<DDB1); //Bits conectados a LEDS
		
	//LEDS a cero
	PORTB &= ~((1<<PORTB0)|(1<<PORTB1));
	PORTD &= ~((1<<PORTD2)|(1<<PORTD3)|(1<<PORTD4)|(1<<PORTD5)|(1<<PORTD6)|(1<<PORTD7));
	
	//Configuracion de ADC
	ADMUX |= (1 << ADLAR); 
	ADMUX = (1 << REFS0); // AVcc como referencia. Quitar la configuracion de ADLAR cuando se usen servos
	ADCSRA = (1 << ADEN) | (1 << ADPS2) | (1 << ADPS1) | (1 << ADPS0); // Prescaler 128
	
	// Salida PWM en D11 (OC2A)
	DDRB |= (1 << DDB3);
	// Fast PWM, no invertido
	TCCR2A = (1 << COM2A1) | (1 << WGM21) | (1 << WGM20);
	TCCR2B = (1 << CS21);   // Prescaler 8 (~7.8kHz)

	OCR2A = 0;  // Motor apagado al inicio
	
	I2C_Slave_Init(SlaverAddress); // Se define como slave con la direccion SlaveAddress
	
	sei();	//Habilitar interrupciones
	
	while (1)
	{
		/*if(buffer == 'R'){
			PIND |= (1<<PIND2);
			buffer = 0;
		}*/
		//refreshPORT(buffer);
		
		//Inicializamos secuencia de adc
		Foto1 = LecturaADC(0);
		Foto2 = LecturaADC(1);
		Foto3 = LecturaADC(2);
		
		valorADC = ((uint16_t)Foto1+Foto2+Foto3)/3;
		
		if (buffer & 0x01){ //Aqui se determina si el maestro esta pidiendo encender el relay
			PORTD |= (1<<PORTD3);
		}else{	
			PORTD &= ~(1<<PORTD3); //sino se apaga el relay
		}
		
		OCR2A = buffer;
		//escrituraMotorDC(valorADC);
		//refreshPORT(valorADC);
		_delay_ms(20);
		
	}
}


ISR(TWI_vect){
	uint8_t estado = TWSR & 0xFC; // Nos quedamos unicamente con los bist de estado TWI Status
	switch(estado){
		// Slave debe de recibir dato
		case 0x60: //SLA+W recibido
		case 0x70: // General call
		TWCR = (1<<TWINT)|(1<<TWEN)|(1<<TWIE)|(1<<TWEA);
		break;
		
		case 0x80: // Dato recibido, ACK enviado
		case 0x90: // Dato recibido General Call, ACK enviado
		buffer = TWDR;
		TWCR = (1<< TWINT)|(1<<TWEN)|(1<<TWIE)|(1<<TWEA);
		break;
		
		//Slave debe transmitir dato
		case 0xA8: // SLA+R recibido
		case 0xB8: // Dato transmitido, ACK recibido
		TWDR = valorADC; // dato a enviar
		TWCR = (1<<TWINT)|(1<<TWEN)|(1<<TWIE)|(1<<TWEA);
		break;
		
		case 0xC0: //Dato transmitido, NACK recibido
		case 0xC8: // Ultimo dato transmitido
		TWCR = 0;
		TWCR = (1<<TWEN)|(1<<TWEA)|(1<<TWIE);
		break;
		
		case 0xA0: // STOP o repeated START recibido como slave
		TWCR = (1<<TWINT)|(1<<TWEN)|(1<<TWIE)|(1<<TWEA);
		break;
		// Cualquier error
		default:
		TWCR = (1<<TWINT)|(1<<TWEN)|(1<<TWIE)|(1<<TWEA);
		break;
	}
}
