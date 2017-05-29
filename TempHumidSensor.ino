#include "Sensor.h"

/* DHT Based temperature sensor */

TemperatureSensor::TemperatureSensor( const String& id, const DHT& dht ) : Sensor( id ), d_dht(dht) {
}

float TemperatureSensor::getValue()  {
  float val= d_dht.readTemperature();
   if( isnan( val ) ) {
    return 0.0;
  } else {
    return val;
  }
}


/* DHT Based humidity sensor */

HumiditySensor::HumiditySensor( const String& id, const DHT& dht ) : Sensor( id ), d_dht(dht) {
}

float HumiditySensor::getValue()  {
  float val =  d_dht.readHumidity();
  if( isnan( val ) ) {
    return 0.0;
  } else {
    return val;
  }
}
