/* Base class - handles fundamental sensor operations such as sending the data to the Wemos over
    serial.
*/

#include "Sensor.h"

Sensor::Sensor(const String& id) : d_id(id) {


}

void  Sensor::setup()
{


}

void  Sensor::update()
{


}

void Sensor::sendData(Stream& ser)
{
  ser.write(4);
  ser.write("DATA");
}

