#include "Sensor.h"


#define VOLTAGE_DIVIDER_FACTOR 4.0

#define VOLTAGE_PIN A0
#define CURRENT_PIN A1

#define UPDATE_FREQUENCY 50   // Only update every 50 calls (each 50th time through loop() )



// Current sensor - based on  ACS714  current sensor
CurrentSensor::CurrentSensor( const String& id) : Sensor( id ) {

}

void CurrentSensor::setup() {
  pinMode( CURRENT_PIN, INPUT );
}

float CurrentSensor::getValue()  {
  // We want at least 10 values to average so if we don't have them now then force an update:
  if ( d_num_values < 10 ) {
    for ( int i = 0; i < 10; i++ ) {
      update(true);
    }
  }
  return d_accumulation / (float) d_num_values;
}

void CurrentSensor::update(bool force_update) {
  if ( !force_update) {
    d_accum_cnt ++;
    if ( d_accum_cnt % UPDATE_FREQUENCY != 0 ) {
      return;
    }
  }
  int vin = analogRead(CURRENT_PIN);
  // Convert to current. First subtract the midpoint of 2.5, then divide by 0.185 to
  // convert volts to amps:
  d_accumulation += ((5.0 * vin / 1023.0) - 2.5) / 0.185;
  d_num_values++;
}



void CurrentSensor::resetAverages()
{
  d_num_values = 0;
  d_accumulation = 0.0;
  d_accum_cnt = 0;
}

BatteryVoltageSensor::BatteryVoltageSensor( const String& id) : Sensor( id ) {

}

void BatteryVoltageSensor::setup() {
  pinMode( VOLTAGE_PIN, INPUT );
}

float BatteryVoltageSensor::getValue()   {
   // We want at least 10 values to average so if we don't have them now then force an update:
  if ( d_num_values < 10 ) {
    for ( int i = 0; i < 10; i++ ) {
      update(true);
    }
  }
  return d_accumulation / (float) d_num_values;
}

void BatteryVoltageSensor::update(bool force_update) {
  if ( ! force_update ) {
    d_accum_cnt ++;
    if ( d_accum_cnt % UPDATE_FREQUENCY != 0 ) {
      return;
    }
  }
  int vin = analogRead(CURRENT_PIN);
  d_accumulation += ((5.0 * vin / 1023.0) * VOLTAGE_DIVIDER_FACTOR);
  d_num_values++;
}

void BatteryVoltageSensor::resetAverages() {
  d_num_values = 0;
  d_accumulation = 0.0;
  d_accum_cnt = 0;
}

