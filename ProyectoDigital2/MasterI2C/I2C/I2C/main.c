/*
 * I2C.c
 *
 * Created: 5/2/2026 10:47:43
 * Author : santi
 */ 

#define F_CPU 16000000
#include <avr/io.h>
#include <util/delay.h>
#include <stdlib.h>
#include <stdbool.h>
#include "I2C.h"
#include "DISPLAYLCD.h"
#include "ADC.h"

#define  slave1 0x30
#define  slave2 0x40
#define AHT10_ADDR  0x38

#define  slave1R (0x30 << 1) | 0x01
#define  slave1W (0x30 << 1) & 0b11111110
#define  slave2R (0x40 << 1) | 0x01
#define  slave2W (0x40 << 1) & 0b11111110
#define AHT10_W  (AHT10_ADDR << 1) & 0b11111110
#define AHT10_R  (AHT10_ADDR << 1) | 0x01

uint8_t bufferI2C = 0;
volatile uint8_t adc_i2c = 0;   // dato a enviar

uint8_t DATAenvioSl2 = 0;
uint8_t DATArecibirSl2 = 0;

float temperatura = 0;
bool calentar = false;
bool enfriar = false;

// ===== ISR ADC =====
ISR(ADC_vect){
      //adc_i2c = ADC >> 2;   // 10 bits ? 8 bits
      ADCSRA |= (1<<ADSC);  // siguiente conversión
}


int main(void)
{
      init8bits();
      
      // Puerto led
      DDRB |= (1<<DDB5);
      PORTB &= ~(1<< PORTB5); // Iniciar led en 0
    
      
      // ===== I2C =====
      I2C_Master_Init(10000,1); // inicializar como Master Fscl 100KHz, prescaler1
      char texto[16];
      char texto2[16];
      
      // ===== ADC en PC3 (ADC3) =====
      init_ADC(0,128,3);
      
    while (1) 
    {
            //Leer el sensor de temperatura
            if(!I2C_Master_Start()) continue;

            if(!I2C_Master_Write(AHT10_W)){
                  I2C_Master_Stop();
                  continue;
            }

            I2C_Master_Write(0xAC);
            I2C_Master_Write(0x33);
            I2C_Master_Write(0x00);

            I2C_Master_Stop();
            _delay_ms(80);  // tiempo de medición
            
            LogicaDeControl();
            PORTB |= (1<< PORTB5);
            
            //Mandar dato a Slave1
            if(!I2C_Master_Start()) continue;
            if(!I2C_Master_Write(slave1W)){ //Direccion slave
                  I2C_Master_Stop();
                  continue;}
            I2C_Master_Write(adc_i2c);
            I2C_Master_Stop();
            _delay_ms(10);
            
            //Recibir dato de Slave1
            if(!I2C_Master_Start()) continue;
            if(!I2C_Master_Write(slave1W)){ //Direccion slave
                  I2C_Master_Stop();
                  continue;}
            if(!I2C_Master_RepeatedStart()) {
                  I2C_Master_Stop();
                  continue;}
            if(!I2C_Master_Write(slave1R)){ //Direccion slave
                  I2C_Master_Stop();
                  continue;}
            I2C_Master_Read(&bufferI2C, 0); // Dato
            I2C_Master_Stop();
            
            //Mandar dato a Slave2
            if(!I2C_Master_Start()) continue;
            if(!I2C_Master_Write(slave2W)){ //Direccion slave
                  I2C_Master_Stop();
            continue;}
            I2C_Master_Write(DATAenvioSl2); // Dato
            I2C_Master_Stop();
            _delay_ms(10);
            
            //Recibir dato de Slave2
            if(!I2C_Master_Start()) continue;
            if(!I2C_Master_Write(slave2W)){ //Direccion slave
                  I2C_Master_Stop();
            continue;}
            if(!I2C_Master_RepeatedStart()) {
                  I2C_Master_Stop();
            continue;}
            if(!I2C_Master_Write(slave2R)){ //Direccion slave
                  I2C_Master_Stop();
            continue;}
            I2C_Master_Read(&DATArecibirSl2, 0); // Dato
            I2C_Master_Stop();
            
            
            uint8_t data[6];

            if(!I2C_Master_Start()) continue;

            if(!I2C_Master_Write(AHT10_R)){
                  I2C_Master_Stop();
                  continue;
            }

            for(uint8_t i=0; i<5; i++){
                  I2C_Master_Read(&data[i], 1); // ACK
            }

            I2C_Master_Read(&data[5], 0); // NACK último byte

            I2C_Master_Stop();

            
            // Convertir números a texto
            itoa(bufferI2C, texto, 10);
            itoa(DATArecibirSl2, texto2, 10);
            
            uint32_t temp_raw;
            temp_raw = ((uint32_t)(data[3] & 0x0F) << 16) |
                           ((uint32_t)data[4] << 8) |
                           data[5];

            temperatura = ((temp_raw * 200.0) / 1048576.0) - 50;
            char tempString[10];
			char tempString2[10];
			
            // ----- Conversión a string -----
            dtostrf(temperatura, 6, 2, tempString);
            dtostrf(adc_i2c, 6, 2, tempString2);
            // Mostrar en LCD
            LCDCMD(0x01);              // limpiar LCD
            LCD_SET_CURSOR(1,1);
            LCD_WRITE_STRING("Luz:");
            LCD_WRITE_STRING(texto);
            LCD_WRITE_STRING("  ");
            LCD_WRITE_STRING("Fan:");
            LCD_WRITE_STRING(tempString2);
            LCD_SET_CURSOR(1,2);
            LCD_WRITE_STRING("Tempe:");
            LCD_WRITE_STRING(tempString);
            _delay_ms(200);


      }
}

void LogicaDeControl(){
      //bufferI2C manda la cantidad de luz (<=20 es de noche)
      //temperatura es la verdadera temperatura (tratar de mantener)
      //adc_i2c se manda el dato de velocidad de ventilador con el primer bit para relay
      //DATArecibirSl2 es el dato que mandara el segundo slave que solo dice si tienen comida o no
      
      //Aqui se determina si hay que enfriar o calentar los pollitos
      if (temperatura >= 28.5) {
            enfriar = true;
            calentar = false;
      }
      else if (temperatura <= 26.5) {   // <- diferente punto para apagar
            enfriar = false;
      }

      if (temperatura <= 23.5) {
            calentar = true;
            enfriar = false;
      }
      else if (temperatura >= 25.5) {   // <- diferente punto para apagar
            calentar = false;
      }
      
      //La condicion mas importante es si es de noche o de dia
      if (bufferI2C <= 20){ //Aqui se escribe el valor umbral en el que se cambia de dia a noche
                  //Es de noche
                  if(enfriar){
                        //Destapar pollos
                        DATAenvioSl2 = 0b00000000;
                        adc_i2c = 0b00001110;
                  }else if(calentar){
                        //Tapar Pollos
                        DATAenvioSl2 = 0b00001111;
						adc_i2c = 0b00000000;
                  }else{
                        adc_i2c = 0x00;
                        //No cambia como esten tapados
                  }
                  
            }else{
                  //Es de dia?
                  if(enfriar){
                        //Destapar pollos
                        DATAenvioSl2 = 0b00000000;
                        if(temperatura>=29){
                              adc_i2c = 0b10000110;
                        }else{
                              adc_i2c = 0b01101110;}
                        }else if(calentar){
                        adc_i2c = 0b00000001;
                        }else{
                        adc_i2c = 0x00;
                        //No cambia como esten tapados
                  }
            }
            
      
      //En caso de emergencia se calientan o se enfrian solo para casos extraordinarios
      if(temperatura <= 20){
            //Calentar de emergencia
            adc_i2c = 0x01; //Se sobreescribe apagando por completo ventilador y encendiendo luces de calor
      }else if(temperatura >= 30){
            //Enfriar de emergencia
            adc_i2c = 0xFE; //Se sobreescribe luces de calor a cero y ventilador a full velocidad
      }
      
      if(DATArecibirSl2==0){
            //servo posicion de abierto
            DATAenvioSl2 = (DATAenvioSl2 & 0x0F) | 0x00;
      }else{
            //Servo posicion de cerrado
             DATAenvioSl2 = (DATAenvioSl2 & 0x0F) | 0xF0;
      }
}