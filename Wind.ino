#include "Sensor.h"


//#define DEBUG_WIND 1  // Comment out to turn off debugging

volatile int wind_speed_count = 0; // Counter used by interrupt



const unsigned long int WIND_SPEED_COUNT_PERIOD= 10*1000l;   // Update windspeed every 10 seconds
#define WIND_SPEED_CONVERSION_FACTOR 2.4  // One rotation per second apparently is 2.4 km/hr 

const int NUM_VOLTAGES = 16;

// Associates voltages and degrees:
class VoltageTableItem {
  public:
    VoltageTableItem( float voltage, float degrees ) : d_voltage(voltage), d_degrees(degrees) {};
  public:
    float d_voltage;
    float d_degrees;
};


static const VoltageTableItem voltage_deg_table[NUM_VOLTAGES] = {
  VoltageTableItem( 3.85, 0.0 ),
  VoltageTableItem( 1.98, 22.5 ),
  VoltageTableItem( 2.25, 45.0 ),
  VoltageTableItem( 0.41, 67.5 ),
  VoltageTableItem( 0.45, 90.0 ),
  VoltageTableItem( 0.32, 112.5 ),
  VoltageTableItem( 0.90, 135.0),
  VoltageTableItem( 0.62, 157.5),
  VoltageTableItem( 1.40, 180.0),
  VoltageTableItem( 1.19, 202.5),
  VoltageTableItem( 3.08, 225.0),
  VoltageTableItem( 2.93, 247.5),
  VoltageTableItem( 4.62, 270.0),
  VoltageTableItem( 4.04, 292.5),
  VoltageTableItem( 4.78, 315.0),
  VoltageTableItem( 3.43, 337.5 )
};


WindSpeedSensor::WindSpeedSensor( const String& id, const int& pin) : Sensor(id), d_pin(pin) {


}

void WindSpeedSensor::setup() {
  // Setup the pin and attach the interrupt:

  pinMode( d_pin, INPUT_PULLUP );
  attachInterrupt( digitalPinToInterrupt(d_pin), windSpeedUpdate, FALLING );
  d_update_time = millis();

}

void WindSpeedSensor::update(bool) {
  // Calculate the windspeed based on the number of rotation counts in this period:

  unsigned long int elapsed_time = millis() - d_update_time;
  if ( elapsed_time > WIND_SPEED_COUNT_PERIOD ) {
    detachInterrupt(digitalPinToInterrupt(d_pin));

    float hz = (wind_speed_count/4.0) / (WIND_SPEED_COUNT_PERIOD / 1000.0); // four pulses per rotation
    d_wind_speed_value = WIND_SPEED_CONVERSION_FACTOR * hz;

#ifdef DEBUG_WIND
    Serial.print("Updating Anamometer - count = ");
    Serial.print(wind_speed_count );
    Serial.print(" ; speed (kph) = ");
    Serial.println( d_wind_speed_value );
#endif

    wind_speed_count = 0;

    attachInterrupt( digitalPinToInterrupt(d_pin), windSpeedUpdate, FALLING );
    d_update_time += WIND_SPEED_COUNT_PERIOD;
  }

}

void WindSpeedSensor::windSpeedUpdate() {
  // Update the count
  wind_speed_count ++;

}

float WindSpeedSensor::getValue()  {
  return d_wind_speed_value;
}

/************************************************* Wind direction ********************************************/

// Wind direction sensor - reads voltage off A2 and converts to degrees via table lookup
WindDirectionSensor::WindDirectionSensor( const String& id, const int& pin ) : AveragingSensor(id), d_pin(pin) {

}

void WindDirectionSensor::setup()
{
  pinMode( d_pin, INPUT );
}


float WindDirectionSensor::readSensor()
{
  float voltage = (analogRead(d_pin) / 1023.0) * 5.0;

  return voltageToDegrees( voltage );

}

float WindDirectionSensor::voltageToDegrees( float voltage ) {
  // Find the closest match to the voltage values in the table:
  float smallest_diff = 1000.0;
  float ret_val = 0.0;

  for ( int i = 0; i < NUM_VOLTAGES; i++ ) {
    float diff = fabs(voltage_deg_table[i].d_voltage - voltage );
    if ( diff < smallest_diff ) {
      ret_val = voltage_deg_table[i].d_degrees;
      smallest_diff = diff;
    }

  }

#ifdef DEBUG_WIND
//  Serial.print("Wind direction sensor - voltage = ");
//  Serial.print(voltage);
//  Serial.print(" degrees = ");
//  Serial.println( ret_val );
#endif

  return ret_val;

}

