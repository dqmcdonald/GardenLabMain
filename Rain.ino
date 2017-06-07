//#include "Sensor.h"


//#define DEBUG_RAIN 1

const unsigned long int RAINFALL_COUNT_PERIOD = 60 * 1000l; // Update rainfall every 60 seconds

volatile int rainfall_count = 0;  // Incremented in the interrupt function

// Tipping Bucket Rainfall Sensor


RainfallSensor::RainfallSensor( const String& id, const int& pin ) : Sensor(id), d_pin(pin)
{


}

void RainfallSensor::setup() {
  pinMode(d_pin, INPUT_PULLUP);
  attachInterrupt( digitalPinToInterrupt(d_pin), rainfallUpdate, FALLING );
  d_update_time = millis();
}

void RainfallSensor::resetAccumulation() {
  d_accumulated_rainfall = 0.0;
}

void RainfallSensor::update(bool) {

  unsigned long int elapsed_time = millis() - d_update_time;
  if ( elapsed_time > RAINFALL_COUNT_PERIOD ) {
    detachInterrupt(digitalPinToInterrupt(d_pin));

    d_accumulated_rainfall = 0.2794 * (float)rainfall_count; // Each switch closure is 0.2794mm

#ifdef DEBUG_RAIN
    Serial.print("Updating rainfall - count = ");
    Serial.print(rainfall_count );
    Serial.print(" accujulated rainfall (mm) = ");
    Serial.println( d_accumulated_rainfall );
#endif

    rainfall_count = 0;

    attachInterrupt( digitalPinToInterrupt(d_pin), rainfallUpdate, FALLING );
    d_update_time += RAINFALL_COUNT_PERIOD;

  }
}
float RainfallSensor::getValue() {
  return d_accumulated_rainfall;
}

void RainfallSensor::rainfallUpdate()
{
  rainfall_count++;
}


