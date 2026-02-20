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

//#define  SlaverAddress 0x30
#define  SlaverAddress 0x40

uint8_t buffer = 0;
volatile uint8_t valorADC =0;
uint8_t Foto1 = 0;
uint8_t Foto2 = 0;
uint8_t Foto3 = 0;
uint16_t Prom = 0;
uint8_t DiaNoche = 0;
uint8_t BloqueoSentido = 0;

// Definir números de bit directamente
#define STEP_BIT 2   // PD2
#define DIR_BIT  3   // PD3

void initADC(void);

//Funcion de control del stepper
void stepper_step(int steps, int dir)
{
    // Dirección
    if(dir)
        PORTD |= (1 << DIR_BIT);
    else
        PORTD &= ~(1 << DIR_BIT);

    // Pulsos STEP
    for(int i = 0; i < steps; i++)
    {
        PORTD |= (1 << STEP_BIT);
        _delay_us(25);
        PORTD &= ~(1 << STEP_BIT);
        _delay_us(25);
    }
}

int main(void)
{
	DDRD |= (1<<DDD2);
	PORTD &= ~(1<<PORTD2);
	
	DDRD |= (1<<DDD2)|(1<<DDD3)|(1<<DDD4)|(1<<DDD5)|(1<<DDD6)|(1<<DDD7);
	DDRB |= (1<<DDB0)|(1<<DDB1); //Bits conectados a LEDS
	
	//Se configura como entrada donde conecta el sensor de presencia
	DDRB &= ~(1 << DDB5);
	
	//Pines que controlan modulo de stepper
	 DDRD |= (1 << STEP_BIT) | (1 << DIR_BIT);
	
	//LEDS a cero
	PORTB &= ~((1<<PORTB0)|(1<<PORTB1));
	PORTD &= ~((1<<PORTD2)|(1<<PORTD3)|(1<<PORTD4)|(1<<PORTD5)|(1<<PORTD6)|(1<<PORTD7));
	
	 // D5 y D6 como entrada
    DDRD &= ~((1 << DDD4) | (1 << DDD6));

    // Activar pull-up interno
    PORTD |= (1 << PORTD4) | (1 << PORTD6);
	
	// Timer1 – Servo 1
    DDRB |= (1 << DDB1) | (1 << DDB2);  // D9 (OC1A) y D10 (OC1B)
    TCCR1A = (1 << COM1A1) | (1 << COM1B1) | (1 << WGM11);
    TCCR1B = (1 << WGM13) | (1 << WGM12) | (1 << CS11); // Prescaler 8
    ICR1 = 40000; // 20ms -> TOP = 40000 (16MHz / 8 / 50Hz)
	
	//Configuracion de ADC
	/*ADMUX |= (1 << ADLAR); 
	ADMUX = (1 << REFS0); // AVcc como referencia. Quitar la configuracion de ADLAR cuando se usen servos
	ADCSRA = (1 << ADEN) | (1 << ADPS2) | (1 << ADPS1) | (1 << ADPS0); // Prescaler 128
	
	// Salida PWM en D11 (OC2A)
	DDRB |= (1 << DDB3);
	// Fast PWM, no invertido
	TCCR2A = (1 << COM2A1) | (1 << WGM21) | (1 << WGM20);
	TCCR2B = (1 << CS21);   // Prescaler 8 (~7.8kHz)

	OCR2A = 0;  // Motor apagado al inicio*/
	
	I2C_Slave_Init(SlaverAddress); // Se define como slave con la direccion SlaveAddress
	
	sei();	//Habilitar interrupciones
	
	while (1)
	{
		
		//Se lee si el plato de comida tiene alimento y se manda al maestro
		if (PINB & (1 << PINB5)){
			valorADC = 1;
		}else{
			valorADC = 0;}
		
		// Si D5 se presiona (va a 0)
        if (!(PIND & (1 << PIND4)))
        {
            BloqueoSentido = 0;}

        // Si D6 se presiona (va a 0)
        if (!(PIND & (1 << PIND6)))
        {
            BloqueoSentido = 1;}
		
		// ----- DISPOSITIVO A (bits altos) -----
		if ((buffer & 0xF0) == 0x00) {
			// 0000xxxx ? Encender A
			ServoTimer1(1);
		}
		else if ((buffer & 0xF0) == 0xF0) {
			// 1111xxxx ? Apagar A
			ServoTimer1(0);
		}

		// ----- DISPOSITIVO B (bits bajos) -----
		if ((buffer & 0x0F) == 0x0F) {
			// xxxx1111 ? Encender B
			if(BloqueoSentido == 0){
			stepper_step(20, 1);}
			//_delay_ms(500);
		}
		else if ((buffer & 0x0F) == 0x00) {
			// xxxx0000 ? Apagar B
			if(BloqueoSentido == 1){
			stepper_step(20, 0);}
			//_delay_ms(500);
		}
		
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
