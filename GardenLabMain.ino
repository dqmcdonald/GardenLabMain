/*
   Main controller for "GardenLab" observatory/weather station. This Arduino Mega 2560 sketch supports
   the following operations:
    Monitoring solar battery system voltage and current draw
    Measure:
        Temperature
        Humidity
        Barometric Pressure
        Wind Speed
        Wind Direction
        Rain fall

    Send data to Wemos D1 Mini to be forwarded over WIFI to Raspberry Pi based server
    Display current settinsg on serial based LCD display


    Quentin McDonald
    May 2017
*/

#include <DHT.h>
#include <Bounce2.h>

#include "LCD.h"



/* Pin assignments follow:

  A0  - Battery voltage monitoring through 4-1 Voltage divider
  A1  - Current load monitoring

  D16 - LCD Serial comms
  D18 - Serial1 TX (for Wemos D1 Mini comms)
  D19 - Serial1 RX (for Wemos D1 Mini comms)

  D49 - "LCD display mode" pushbutton
*/


#define MODE_BUTTON_PIN 49


/* Global Variables */

SerialLCD lcd;

Bounce mode_button = Bounce();

int current_mode = 0;


void setup() {
  Serial.begin(9600);


  // Mode button:
  pinMode(MODE_BUTTON_PIN, INPUT_PULLUP);
  mode_button.attach(MODE_BUTTON_PIN);
  mode_button.interval(20);



  // LCD Setup:
  lcd.clear();
  lcd.backlight(20);
  lcd.displayLine(2, (char*)"   GardenLab v1.0");
  delay(5000);
  lcd.clear();


}

void loop() {

  mode_button.update();
  if ( mode_button.fell() ) {
    // Increment mode

  }


}

