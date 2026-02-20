// Adafruit IO Publish & Subscribe Example
//
// Adafruit invests time and resources providing this open source code.
// Please support Adafruit and open source hardware by purchasing
// products from Adafruit!
//
// Written by Todd Treece for Adafruit Industries
// Copyright (c) 2016 Adafruit Industries
// Licensed under the MIT license.
//
// All text above must be included in any redistribution.

/************************** Configuration ***********************************/

// edit the config.h tab and enter your Adafruit IO credentials
// and any additional configuration needed for WiFi, cellular,
// or ethernet clients.
#include "config.h"

/************************ Example Starts Here *******************************/

// this int will hold the current count for our sketch
uint8_t temperatura = 0;
uint8_t ventilador = 0;
bool luces = 0;
bool cortina = 0;
bool comida = 0;
// Track time of last published messages and limit feed->save events to once
// every IO_LOOP_DELAY milliseconds.
//
// Because this sketch is publishing AND subscribing, we can't use a long
// delay() function call in the main loop since that would prevent io.run()
// from being called often enough to receive all incoming messages.
//
// Instead, we can use the millis() function to get the current time in
// milliseconds and avoid publishing until IO_LOOP_DELAY milliseconds have
// passed.
#define IO_LOOP_DELAY 5000
unsigned long lastUpdate = 0;

// ===== UART =====
#define RXD2 7   // D7
#define TXD2 6   // D6

// FEEDS
AdafruitIO_Feed *temperaturaFeed = io.feed("temperatura");
AdafruitIO_Feed *ventiladorFeed = io.feed("ventilador");
AdafruitIO_Feed *lucesFeed = io.feed("luces");
AdafruitIO_Feed *cortinaFeed = io.feed("cortina");
AdafruitIO_Feed *comidaFeed = io.feed("comida");


void setup() {

  // start the serial connection
  Serial.begin(115200);
  Serial1.begin(9600, SERIAL_8N1, RXD2, TXD2);
  // wait for serial monitor to open
  //while(! Serial);

  Serial.print("Connecting to Adafruit IO");

  // connect to io.adafruit.com
  io.connect();

  // set up a message handler for the count feed.
  // the handleMessage function (defined below)
  // will be called whenever a message is
  // received from adafruit io.
  ventiladorFeed->onMessage(handleVentilador);
  comidaFeed->onMessage(handleComida);
  cortinaFeed->onMessage(handleCortina);
  lucesFeed->onMessage(handleLuces);
  // wait for a connection
  while(io.status() < AIO_CONNECTED) {
    Serial.print(".");
    delay(500);
  }

  // we are connected
  Serial.println();
  Serial.println(io.statusText());
  
  ventiladorFeed->get();
  comidaFeed->get();
  cortinaFeed->get();
  lucesFeed->get();
}

void loop() {

  // io.run(); is required for all sketches.
  // it should always be present at the top of your loop
  // function. it keeps the client connected to
  // io.adafruit.com, and processes any incoming data.
  io.run();

  // ===== RECIBIR TEMPERATURA DEL NANO =====
  if(Serial1.available()){
    String msg = Serial1.readStringUntil('\n');

    if(msg.startsWith("T:")){
      temperatura = msg.substring(2).toInt();
      temperaturaFeed->save(temperatura);

      Serial.print("Temp recibida: ");
      Serial.println(temperatura);
    }
  }
}

// ===== HANDLERS ADAFRUIT → UART =====

void handleVentilador(AdafruitIO_Data *data){
  ventilador = data->toInt();
  Serial1.print("F:");
  Serial1.println(ventilador);

  ventilador = atoi(data->value()); 
  Serial.print("Ventilador <- "); 
  Serial.println(ventilador);
}

void handleComida(AdafruitIO_Data *data){
  comida = data->toInt();
  Serial1.print("M:");
  Serial1.println(comida);

  comida = atoi(data->value()); 
  Serial.print("Comida <- "); 
  Serial.println(comida);
}

void handleCortina(AdafruitIO_Data *data){
  cortina = data->toInt();
  Serial1.print("C:");
  Serial1.println(cortina);

  cortina = atoi(data->value()); 
  Serial.print("Cortina <- "); 
  Serial.println(cortina);
}

void handleLuces(AdafruitIO_Data *data){
  luces = data->toInt();
  Serial1.print("L:");
  Serial1.println(luces);

  luces = atoi(data->value()); 
  Serial.print("Luces <- "); 
  Serial.println(luces);
}