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

    Send data to Wemos D1 Mini to be forwarded over WIFI to Raspberry Pi based server. D1 Mini is only turned on when it's needed,
    the Arduino controls it's power via a MOSFET
    Display current settings on serial based LCD display


    Quentin McDonald
    May 2017
*/


#include <Bounce2.h>

#include "LCD.h"
#include "Sensor.h"


#define DATA_SEND_PERIOD (1000l*60*5)    // Send data every 5 minutes
#define LCD_UPDATE_PERIOD (1000l*10)     // Update LCD every 10 seconds
#define D1_WARM_UP_PERIOD (1000l*5)      // Allow a very generous 5 seconds for D1 to warm up
#define D1_ON_PERIOD (1000l*5)           // Allow 5 seconds for D1 to send data and shutdown

/* Utility functions */
// Send a string value via Serial
bool send_string( Stream& ser, const String& str);

/* Pin assignments follow:

  A0  - Battery voltage monitoring through 4-1 Voltage divider
  A1  - Load current monitoring
  A2  - Wine vane
  A3  - Panel current monitoring

  D2 -  Anemometer (Interrupt)
  D3 -  Rainfall gauge (Interrupt)
  D4  - DHT temp and humidity sensor

  D16 - LCD Serial comms
  D18 - Serial1 TX (for Wemos D1 Mini comms)
  D19 - Serial1 RX (for Wemos D1 Mini comms)
  D20 - BMP pressure + temp sensor (SDA)
  D21 - BMP pressure + temp sensor (SCL)

  D47 - Turn on and off the ESP8266 Wemos D1 Mini via Mosfet on power supply
  D49 - "LCD display mode" pushbutton
*/

#define DHT_PIN 4
#define WIND_SPEED_PIN 2
#define RAINFALL_PIN 3
#define MODE_BUTTON_PIN 49
#define D1_POWER_PIN 47

#define BATTERY_VOLTAGE_PIN A0
#define LOAD_CURRENT_PIN A1
#define WIND_DIRECTION_PIN A2
#define PANEL_CURRENT_PIN A3


/* Enums for D1 power mode */
enum class D1PowerMode { OFF, WARMING_UP, ON };
D1PowerMode d1_power_mode = D1PowerMode::OFF;


/* Mode definitions */
#define LCD_WEATHER_MODE 0
#define LCD_WIND_RAIN_MODE 1
#define LCD_VOLT_CURRENT_MODE 2




// When we sent data
long unsigned int time_at_d1_update = 0;
// When we update the LCD
long unsigned int time_at_lcd_update = 0;

/* Global Variables */

SerialLCD lcd;

Bounce mode_button = Bounce();

int current_mode = LCD_WEATHER_MODE;
const int NUM_MODES = 3;

#define DHTTYPE DHT22   // DHT 22  (AM2302)
DHT dht(DHT_PIN, DHTTYPE);

Adafruit_BMP280 bmp; // I2C

/* Sensors: */

const int NUM_SENSORS = 10;

TemperatureSensor temp_sensor = TemperatureSensor("TEMP", dht );
HumiditySensor humidity_sensor = HumiditySensor("HUMI", dht );
InsideTempSensor itemp_sensor = InsideTempSensor("ITMP", bmp );
PressureSensor pressure_sensor = PressureSensor("PRES", bmp );
CurrentSensor load_current_sensor = CurrentSensor("LCUR", LOAD_CURRENT_PIN);
BatteryVoltageSensor bat_voltage_sensor = BatteryVoltageSensor("BATV", BATTERY_VOLTAGE_PIN);
WindSpeedSensor wind_speed_sensor = WindSpeedSensor("WSPD", WIND_SPEED_PIN);
WindDirectionSensor wind_direction_sensor = WindDirectionSensor("WDIR", WIND_DIRECTION_PIN);
RainfallSensor rainfall_sensor = RainfallSensor("RAIN", RAINFALL_PIN );
CurrentSensor panel_current_sensor = CurrentSensor("PCUR", PANEL_CURRENT_PIN);

// An array of sensors - so we can iterate through them all and perform update/reset etc on them
Sensor* all_sensors[NUM_SENSORS] = {
  &temp_sensor,
  &humidity_sensor,
  &itemp_sensor,
  &pressure_sensor,
  &load_current_sensor,
  &bat_voltage_sensor,
  &wind_speed_sensor,
  &wind_direction_sensor,
  &rainfall_sensor,
  &panel_current_sensor
};


void setup() {
  Serial.begin(9600);
  Serial1.begin(9600);

  // Mode button:
  pinMode(MODE_BUTTON_PIN, INPUT_PULLUP);
  mode_button.attach(MODE_BUTTON_PIN);
  mode_button.interval(20);

  // ESP8266 Power:
  pinMode(D1_POWER_PIN, OUTPUT);
  digitalWrite( D1_POWER_PIN, LOW );


  if (!bmp.begin()) {
    Serial.println("Could not find a valid BMP280 sensor, check wiring!");
  }
  delay(1000);

  // LCD Setup:
  lcd.clear();
  lcd.backlight(15);
  lcd.displayLine(2, (char*)"   GardenLab v1.0");
  delay(5000);
  lcd.clear();

  for ( auto sensor : all_sensors ) {
    sensor->setup();
  }

  time_at_d1_update = millis();
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


  handle_d1_send_data();




  // Check if it's time to update the LCD
  unsigned long int elapsed_time = millis() - time_at_lcd_update;
  if ( elapsed_time >= LCD_UPDATE_PERIOD ) {
    time_at_lcd_update += LCD_UPDATE_PERIOD;

    update_lcd();

  }


}

void handle_d1_send_data() {
  // There are three states for the D1 Mini:
  // "Off" - we are just waiting until it's time to send data to the server, power to the D1 is off.
  // "Warming Up" - the D1 has been turned on, just waiting for it to initialize and connect to the server
  // "On" - the D1 is sending data, wait to give it a chance to do so before shutting down and going back to "Off"

  // Check if it's time to send data to the server:
  unsigned long int elapsed_time = millis() - time_at_d1_update;

  if ( (d1_power_mode == D1PowerMode::OFF) &&  (elapsed_time >= DATA_SEND_PERIOD) ) {
    time_at_d1_update += DATA_SEND_PERIOD;
    d1_power_mode = D1PowerMode::WARMING_UP;
    digitalWrite( D1_POWER_PIN, HIGH );
    Serial.println("Turning on D1 Mini for warm up");
    return;
  }

  if ( (d1_power_mode == D1PowerMode::WARMING_UP) && (elapsed_time >= D1_WARM_UP_PERIOD) ) {

    Serial.println( "D1 Warmed up sending data" );
    // Warmed up - send data to sever:
    send_data_to_server();

    // Reset any accumulating sensors (rainfall etc)
    for ( auto sensor : all_sensors ) {
      sensor->resetAccumulation();
    }

    time_at_d1_update += D1_WARM_UP_PERIOD;
    d1_power_mode = D1PowerMode::ON;
    return;
  }

  if ( (d1_power_mode == D1PowerMode::ON) && (elapsed_time >= D1_ON_PERIOD )) {
    Serial.println("Data sent, shutting down D1 mini" );
    d1_power_mode = D1PowerMode::OFF;
    time_at_d1_update += D1_ON_PERIOD;
    pinMode( D1_POWER_PIN, LOW );
    return;
  }
}

/* Update the LCD display */
void update_lcd()
{

  if ( current_mode == LCD_WEATHER_MODE ) {


    String temperature =   "Temperature:  " + String(temp_sensor.getValue(), 1 );
    lcd.displayLine( 1, temperature );

    String humidity =  "Humidity:     " + String(humidity_sensor.getValue(), 1);
    lcd.displayLine( 2, humidity);

    String pressure =  "Pressue:      " + String(pressure_sensor.getValue() / 100.0, 0);
    lcd.displayLine( 3, pressure);

    String itemp =     "Inside Temp:  " + String(itemp_sensor.getValue(), 1);
    lcd.displayLine(4, itemp);

  } else if ( current_mode ==  LCD_WIND_RAIN_MODE) {


    String windspeed =   "Windspeed (km):" + String(wind_speed_sensor.getValue(), 1 );
    lcd.displayLine( 1, windspeed );

    String wind_dir =    "Wind dir(deg):" + String(wind_direction_sensor.getValue(), 1);
    lcd.displayLine( 2, wind_dir);

    String rainfall =    "Rainfall (mm):" + String(rainfall_sensor.getValue() , 1);
    lcd.displayLine( 3, rainfall);



  } else if ( current_mode == LCD_VOLT_CURRENT_MODE ) {
    String voltage =   "Battery (V):  " + String(bat_voltage_sensor.getValue(), 1 );
    lcd.displayLine( 1, voltage );

    String load_current =   "Load (mA):    " + String(load_current_sensor.getValue() * 1000.0, 0);
    lcd.displayLine( 2, load_current);

    String panel_current =   "Panel (mA):  " + String(panel_current_sensor.getValue() * 1000.0, 0);
    lcd.displayLine( 2, panel_current);
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


  send_string( Serial1, data_string );
  Serial.println("Data string sent");

}



// Send a string value via Serial
bool send_string( Stream& ser, const String& str)
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

}






