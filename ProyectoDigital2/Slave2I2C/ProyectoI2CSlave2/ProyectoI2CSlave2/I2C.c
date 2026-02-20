/*
 * I2C.c
 *
 * Created: 5/2/2026
 *  Author: santi
 */ 

#include "I2C.h"

// Funcion para inicializar I2C Master
void I2C_Master_Init(unsigned long SCL_Clock, uint8_t Prescaler){
	DDRC &= ~((1<<DDC4)|(1<<DDC5));	// Pines de i2c como entradas SDA y SCL
	// Seleccionamos el valor de los bits para el prescaler del registro TWSR
	switch(Prescaler){
		case 1:
			TWSR &= ~((1<<TWPS1)|(1<<TWPS0));
		break;
		case 4:
			TWSR &= ~(1<<TWPS1);
			TWSR |= (1<<TWPS0);
		break;
		case 16:
			TWSR &= ~(1<<TWPS0);
			TWSR |= (1<<TWPS1);
		break;
		case 64:
			TWSR |= (1<<TWPS1)|(1<<TWPS0);
		break;
		default:
			TWSR &= ~((1<<TWPS1)|(1<<TWPS0));
			Prescaler = 1;
		break;
	}
	TWBR = ((F_CPU/SCL_Clock)-16)/(2*Prescaler);	// Calcular lavelocidad
	TWCR |= (1<<TWEN);	// Activar la interface (TWI) I2C
}

// Funcion de inicio de la comunicacion I2C
uint8_t	I2C_Master_Start(void)
{
	TWCR = (1<<TWINT)|(1<<TWSTA)|(1<<TWEN);	// Master, Reiniciar bandera de Int, Condicion de start
	while (!(TWCR & (1<<TWINT)));	// Esperamos a que se encienda la bandera
	
	return ((TWSR & 0xF8) == 0x08);	// Nos quedamos unicamente con los bits de estado TWI Status y revisamos
}

// Funcion de reinicio de la comunicacion I2C
uint8_t I2C_Master_RepeatedStart(void)
{
	TWCR = (1<<TWINT)|(1<<TWSTA)|(1<<TWEN);	// Master, Reiniciar bandera de Int, Condicion de start
	while (!(TWCR & (1<<TWINT)));	// Esperamos a que se encienda la bandera
	
	return ((TWSR & 0xF8) == 0x10);	// Nos quedamos unicamente con los bits de estado TWI Status y revisamos
}

// Funcion de parada de la comunicacion I2C
void I2C_Master_Stop(void){
	TWCR = (1<<TWEN)|(1<<TWINT)|(1<<TWSTO);	// Inicia el envio secuencia parada STOP
	while (TWCR & (1<<TWSTO));	// Esperamos a que el bit se limpie
}

// Funcion de transmision de datos del maestro al esclavo
//esta funcion devolvera un 0 si el esclavo a recibido el dato
uint8_t I2C_Master_Write(uint8_t dato){
	uint8_t estado;
	
	TWDR = dato;	// Cargar el dato
	TWCR = (1<<TWEN)|(1<<TWINT);	// Inicia la secuencia de envio
	
	while(!(TWCR & (1<<TWINT)));	// Espera al flag TWINT
	estado = TWSR & 0xF8;		// Nos quedamos unicamente con los bits de estado TWI Status
	// Verificar si se transmitio una SLA + W con ACK, o un Dato con ACK
	if (estado == 0x18 || estado == 0x28) {
		return 1;
	}else{
		return estado;
	}
}

// Funcion de recepcion de datos enviados por el esclavo al maestro
//esta funcion es para leer los datos que estan en el esclavo
uint8_t I2C_Master_Read(uint8_t *buffer, uint8_t ack){
	uint8_t estado;
	
	if(ack){
		// ACK: quiero mas datos
		TWCR = (1<<TWINT) | (1<<TWEN) | (1<<TWEA);	// Habilitamos Interfase I2C con envio de ACK
		} else{
		//NACK: ultio byte
		TWCR = (1<<TWINT) | (1<< TWEN);		// Habilitamos interfase I2C sin envio de ACK
	}
	
	while (!(TWCR & (1<<TWINT)));	// Espera al flag TWINT
	
	estado = TWSR & 0xF8;	// Nos quedamos unicamente con los bits de estado TWI Status
	// Verificar si se recibio Dato con ACK o sin ACK
	if(ack && estado != 0x50) return 0; // Data recibida, ACK
	if(!ack && estado != 0x58) return 0; // Data recibida, NACK
	
	*buffer = TWDR; // Obtenemos el resultado en el registro de datos
	return 1;
}

// Funcion para inicializar I2C Esclavo
void I2C_Slave_Init(uint8_t address){
	DDRC &= ~((1<<DDC4)|(1<<DDC5));			// Pines de i2c como entradas
	
	TWAR = address << 1;	// Se asigna la direccion que tendra
	// TWAR = (address << 1| 0x01); // Se asigna la direccion que tendra y habilita llamada general
	
	// Se habilita la interfaz, ACK automatico, se habilita la ISR
	TWCR = (1<< TWEA)|(1<<TWEN)|(1<<TWIE);
}

 void refreshPORT(uint8_t valor){
	if(valor & 0b10000000){
		PORTB |= (1<<PORTB1);
	}else{
		PORTB &= ~(1<<PORTB1);	
	}
	if(valor & 0b01000000){
		PORTB |= (1<<PORTB0);
		}else{
		PORTB &= ~(1<<PORTB0);
	}
	if(valor & 0b00100000){
		PORTD |= (1<<PORTD7);
		}else{
		PORTD &= ~(1<<PORTD7);
	}
	if(valor & 0b00010000){
		PORTD |= (1<<PORTD6);
		}else{
		PORTD &= ~(1<<PORTD6);
	}
	if(valor & 0b00001000){
		PORTD |= (1<<PORTD5);
		}else{
		PORTD &= ~(1<<PORTD5);
	}
	if(valor & 0b00000100){
		PORTD |= (1<<PORTD4);
		}else{
		PORTD &= ~(1<<PORTD4);
	}
	if(valor & 0b00000010){
		PORTD |= (1<<PORTD3);
		}else{
		PORTD &= ~(1<<PORTD3);
	}
	if(valor & 0b00000001){
		PORTD |= (1<<PORTD2);
		}else{
		PORTD &= ~(1<<PORTD2);
	}
}

uint8_t LecturaADC(uint8_t PIN){ //Cambiar a uint16_t cuando se vaya a usar servos
	 ADMUX = (ADMUX & 0xF0) | PIN; // Seleccionar canal
	 ADCSRA |= (1 << ADSC); // Iniciar conversión
	 while (ADCSRA & (1 << ADSC)); // Esperar a que termine
	 uint8_t adc8 = ADC >> 2;
	 return adc8; //cambiar a solo ADC cuando se vaya a usar servos
	 
 }
 
 void ServoTimer1(uint8_t posicion)
{
    if(posicion == 0)
    {
        OCR1A = 2000;   // 0 grados
    }
    else
    {
        OCR1A = 3500;   // 180 grados - 4000
        // si quieres 90°, usa 3000
    }
}
 
