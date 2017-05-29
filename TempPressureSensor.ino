// Temperature sensor, Based on BMP280 sensor

#include "Sensor.h"
#include <Wire.h>
#include <SPI.h>
#include <Adafruit_Sensor.h>
#include <Adafruit_BMP280.h>

#define ALTITUDE_CORRECTION 980; // About 80m above sea level


InsideTempSensor::InsideTempSensor( const String& id, const Adafruit_BMP280& bmp ) : Sensor(id), d_bmp(bmp) {


}



float InsideTempSensor::getValue()  {
  return d_bmp.readTemperature();

}


// Pressure sensor, Based on BMP280 sensor
PressureSensor::PressureSensor( const String& id, const Adafruit_BMP280& bmp ) : Sensor(id), d_bmp(bmp)
{

}

float PressureSensor::getValue( )  {
  return d_bmp.readPressure() + ALTITUDE_CORRECTION;
}

