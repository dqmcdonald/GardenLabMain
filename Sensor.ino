/* Base class - handles fundamental sensor operations such as sending the data to the Wemos over
    serial.
*/

#include "Sensor.h"

Sensor::Sensor(const String& id) : d_id(id) {


}
void Sensor::resetAverages()
{

}
void  Sensor::setup()
{


}

void  Sensor::update(bool force_update)
{


}

// Get data for the current sensor. We won't usually have to override this
String Sensor::getDataString()
{
  String to_send = d_id + "=";
  to_send += String(getValue(), 3);
  return to_send;
}



