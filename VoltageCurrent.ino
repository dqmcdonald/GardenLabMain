#include "Sensor.h"


#define VOLTAGE_DIVIDER_FACTOR 4.0




// Current sensor - based on  ACS714  current sensor
CurrentSensor::CurrentSensor( const String& id, const int& pin) : AveragingSensor( id ), d_pin(pin) {

}

void CurrentSensor::setup() {
  pinMode( d_pin, INPUT );
}

float CurrentSensor::readSensor()  {
  int vin = analogRead(d_pin);
  // Convert to current. First subtract the midpoint of 2.5, then divide by 0.185 to
  // convert volts to amps:
  return ((5.0 * vin / 1023.0) - 2.5) / 0.185;
}


BatteryVoltageSensor::BatteryVoltageSensor( const String& id, const int& pin) : AveragingSensor( id ), d_pin(pin) {

}

void BatteryVoltageSensor::setup() {
  pinMode( d_pin, INPUT );
}

float BatteryVoltageSensor::readSensor() {
   int vin = analogRead(d_pin);
  return ((5.0 * vin / 1023.0) * VOLTAGE_DIVIDER_FACTOR);
}

