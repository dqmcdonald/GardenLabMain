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
    Display current settings on serial based LCD display


    Quentin McDonald
    May 2017
*/


#include <Bounce2.h>

#include "LCD.h"
#include "Sensor.h"


/* Behavior defines: */
#define MAX_DATA_SEND_TRIES 3

#define DATA_SEND_PERIOD (1000*20*1)    // Send data every 5 minutes
#define LCD_UPDATE_PERIOD (1000*10)      // Update LCD every 10 seconds


/* Utility functions */
// Send a string value via Serial and wait for an 'OK' response. If no response is received within 'time_out' ms then return false
bool send_string_with_response( Stream& ser, const String& str, long unsigned int time_out = 1000);

/* Pin assignments follow:

  A0  - Battery voltage monitoring through 4-1 Voltage divider
  A1  - Load current monitoring
  A2  - Wine vane

  D2 -  Anemometer (Interrupt)
  D3 -  Rainfall gauge (Interrupt)
  D4  - DHT temp and humidity sensor

  D16 - LCD Serial comms
  D18 - Serial1 TX (for Wemos D1 Mini comms)
  D19 - Serial1 RX (for Wemos D1 Mini comms)
  D20 - BMP pressure + temp sensor (SDA)
  D21 - BMP pressure + temp sensor (SCL)

  D49 - "LCD display mode" pushbutton
*/

#define DHT_PIN 4
#define MODE_BUTTON_PIN 49

/* Mode definitions */
#define LCD_WEATHER_MODE 0
#define LCD_VOLT_CURRENT_MODE 1



// When we sent data
long unsigned int time_at_data_send = 0;
// When we update the LCD
long unsigned int time_at_lcd_update = 0;

/* Global Variables */

SerialLCD lcd;

Bounce mode_button = Bounce();

int current_mode = 0;
const int NUM_MODES = 2;

#define DHTTYPE DHT22   // DHT 22  (AM2302)
DHT dht(DHT_PIN, DHTTYPE);

Adafruit_BMP280 bmp; // I2C

/* Sensors: */

const int NUM_SENSORS = 6;

TemperatureSensor temp_sensor = TemperatureSensor("TEMP", dht );
HumiditySensor humidity_sensor = HumiditySensor("HUMI", dht );
InsideTempSensor itemp_sensor = InsideTempSensor("ITMP", bmp );
PressureSensor pressure_sensor = PressureSensor("PRES", bmp );
CurrentSensor current_sensor = CurrentSensor("LCUR");
BatteryVoltageSensor bat_voltage_sensor = BatteryVoltageSensor("BATV");

// An array of sensors - so we can iterate through them all and perform update/reset etc on them
Sensor* all_sensors[NUM_SENSORS] = {
  &temp_sensor,
  &humidity_sensor,
  &itemp_sensor,
  &pressure_sensor,
  &current_sensor,
  &bat_voltage_sensor
};


void setup() {
  Serial.begin(9600);
  Serial1.begin(9600);

  // Mode button:
  pinMode(MODE_BUTTON_PIN, INPUT_PULLUP);
  mode_button.attach(MODE_BUTTON_PIN);
  mode_button.interval(20);


  if (!bmp.begin()) {
    Serial.println(F("Could not find a valid BMP280 sensor, check wiring!"));
    while (1);
  }


  // LCD Setup:
  lcd.clear();
  lcd.backlight(20);
  lcd.displayLine(2, (char*)"   GardenLab v1.0");
  delay(5000);
  lcd.clear();

  for ( auto sensor : all_sensors ) {
    sensor->setup();
  }

  time_at_data_send = millis();
  time_at_lcd_update = millis();


  Serial.println("Setup done");
}

void loop() {

  mode_button.update();
  if ( mode_button.fell() ) {
    current_mode ++;
    if ( current_mode >= NUM_MODES ) {
      current_mode = 0;
    }
    lcd.clear();
    update_lcd();
  }

  for ( auto sensor : all_sensors ) {
    sensor->update();
  }

  // Check if it's time to send data to the server:
  unsigned long int elapsed_time = millis() - time_at_data_send;
  if ( elapsed_time >= DATA_SEND_PERIOD ) {
    time_at_data_send += DATA_SEND_PERIOD;

    send_data_to_server();

  }

  // Check if it's time to update the LCD
  elapsed_time = millis() - time_at_lcd_update;
  if ( elapsed_time >= LCD_UPDATE_PERIOD ) {
    time_at_lcd_update += LCD_UPDATE_PERIOD;

    update_lcd();

  }


}

/* Update the LCD display */
void update_lcd()
{


#define LCD_WEATHER_MODE 0
#define LCD_VOLT_CURRENT_MODE 1


  if ( current_mode == LCD_WEATHER_MODE ) {


    String temperature =   "Temperature:  " + String(temp_sensor.getValue(), 1 );
    lcd.displayLine( 1, temperature );

    String humidity =  "Humidity:     " + String(humidity_sensor.getValue(), 1);
    lcd.displayLine( 2, humidity);

    String pressure =  "Pressue:      " + String(pressure_sensor.getValue() / 100.0, 0);
    lcd.displayLine( 3, pressure);

    String itemp =     "Inside Temp:  " + String(itemp_sensor.getValue(), 1);
    lcd.displayLine(4, itemp);

  }

  if ( current_mode == LCD_VOLT_CURRENT_MODE ) {
    String voltage =   "Battery (V):  " + String(bat_voltage_sensor.getValue(), 1 );
    lcd.displayLine( 1, voltage );

    String current =    "Load (mA):    " + String(current_sensor.getValue() / 1000.0, 1);
    lcd.displayLine( 2, current);
  }

  // Reset the average values:
  for ( auto sensor : all_sensors ) {
    sensor->resetAverages();
  }



}



/* Collect data from all the sensors and send it to the ESP8266 via serial */
void send_data_to_server()
{

  String data_string = "";
  int i = 0;
  for ( auto sensor : all_sensors ) {
    if ( i != 0 ) {
      data_string += "&";  // Separator between fields
    }
    data_string += sensor->getDataString();
    i++;
  }
  Serial.print("Sending data string: ");
  Serial.println(data_string);

  // Try and send the data three times if it times out:
  for ( int attempt = 0; attempt < MAX_DATA_SEND_TRIES; attempt++ ) {
    if ( send_string_with_response( Serial1, data_string ) ) {
      Serial.println("Data string send sucessfully");
      return;
    }

  }

  Serial.println("Data string send failed!");
}



// Send a string value via Serial and wait for an 'OK' response. If no response is received within 'time_out' ms then return false
bool send_string_with_response( Stream& ser, const String& str, long unsigned int time_out = 1000)
{

  // Send the length of the string first:
  int len = str.length();

  Serial.print("Sending data of length = ");
  Serial.println(len);
  ser.write((char)len);

  // Now send the bytes individually:
  for ( int i = 0; i < len; i++ ) {
    ser.write(str[i]);
  }

  ser.flush();
  // Now wait for response:
  bool done = false;
  String res = "";
  long unsigned int start_time = millis();
  bool timed_out = false;


  while ( !done ) {
    if ( ser.available() ) {
      char c = ser.read();
      res += c;
    }

    if ( res == "OK") {
      Serial.println("Got OK from ESP8266");
      done = true;
    }

    if ( millis() - start_time > time_out ) {
      timed_out = true;
      Serial.println("Timed out from ESP8266");
      done = true;
    }


  }

  return !timed_out;
}






