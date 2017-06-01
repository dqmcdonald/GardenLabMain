/* Base class - handles fundamental sensor operations such as sending the data to the Wemos over
    serial.
*/

#include "Sensor.h"

#define UPDATE_FREQUENCY 50   // Only update averaging sensors every 50 calls (each 50th time through loop() )



Sensor::Sensor(const String& id) : d_id(id) {


}

Sensor::~Sensor() {
  
}
void Sensor::resetAverages()
{

}
void Sensor::resetAccumulation() {
  
}


void  Sensor::setup()
{


}

void  Sensor::update(bool )
{


}

// Get data for the current sensor. We won't usually have to override this
String Sensor::getDataString()
{
  String to_send = d_id + "=";
  to_send += String(getValue(), 3);
  return to_send;
}


AveragingSensor::AveragingSensor( const String& id) : Sensor( id ) {

}

AveragingSensor::~AveragingSensor() {


}
void AveragingSensor::setup() {
  
}


void AveragingSensor::update(bool force_update) {
  // If we aren't forcing an update then only actually accumulate the values every 50 calls:
  if ( ! force_update ) {
    d_accum_cnt ++;
    if ( d_accum_cnt % UPDATE_FREQUENCY != 0 ) {
      return;
    }
  }
  
  d_accumulation += readSensor();
  d_num_values++;
}

void AveragingSensor::resetAverages() {
  d_num_values = 0;
  d_accumulation = 0.0;
  d_accum_cnt = 0;
}

float AveragingSensor::getValue()   {
   // We want at least 10 values to average so if we don't have them now then force an update:
  if ( d_num_values < 10 ) {
    for ( int i = 0; i < 10; i++ ) {
      update(true);
    }
  }
  return d_accumulation / (float) d_num_values;
}








