#include "Sensor.h"


#define VOLTAGE_DIVIDER_FACTOR 4.0

#define VOLTAGE_PIN A0
#define CURRENT_PIN A1



// Current sensor - based on  ACS714  current sensor
CurrentSensor::CurrentSensor( const String& id) : AveragingSensor( id ) {

}

void CurrentSensor::setup() {
  pinMode( CURRENT_PIN, INPUT );
}

float CurrentSensor::readSensor()  {
  int vin = analogRead(CURRENT_PIN);
  // Convert to current. First subtract the midpoint of 2.5, then divide by 0.185 to
  // convert volts to amps:
  return ((5.0 * vin / 1023.0) - 2.5) / 0.185;
}


BatteryVoltageSensor::BatteryVoltageSensor( const String& id) : AveragingSensor( id ) {

}

void BatteryVoltageSensor::setup() {
  pinMode( VOLTAGE_PIN, INPUT );
}

float BatteryVoltageSensor::readSensor() {
   int vin = analogRead(VOLTAGE_PIN);
  return ((5.0 * vin / 1023.0) * VOLTAGE_DIVIDER_FACTOR);
}

