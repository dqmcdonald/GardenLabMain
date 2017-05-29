#ifndef _SENSOR_H_
#define _SENSOR_H_
/* Declarations for all sensor classes */

#include <Wire.h>
#include <SPI.h>
#include <Adafruit_Sensor.h>
#include <Adafruit_BMP280.h>
#include <DHT.h>


class Sensor {

  public:
    Sensor(const String& id);                  // Initialize with four letter ID used to send
    // data

    virtual void  setup();                     // Setup state of pins etc
    virtual float getValue()  = 0;        // Get the current value
    virtual void  update(bool force_update = false);// Do any required periodic updating tasks
                                                
    virtual String  getDataString();           // Return a string in the form ID=VALUE, suitable for sending to the server
    virtual void resetAverages();              // Reset any average values being accumulated

  protected:
    const String d_id;

};


// Temperature sensor, Based on DHT sensor

class TemperatureSensor : public Sensor {

  public:
    TemperatureSensor( const String& id, const DHT& dht );
    float getValue()  override;

  private:
    const DHT& d_dht;
};


// Humidity sensor, Based on DHT sensor
class HumiditySensor : public Sensor {

  public:
    HumiditySensor( const String& id, const DHT& dht );
    float getValue()  override;

  private:
    const DHT& d_dht;
};


// Temperature sensor, Based on BMP280 sensor
class InsideTempSensor : public Sensor {

  public:
    InsideTempSensor( const String& id, const Adafruit_BMP280& bmp );
    float getValue()  override;

  private:
    const Adafruit_BMP280& d_bmp;
};


// Pressure sensor, Based on BMP280 sensor
class PressureSensor : public Sensor {

  public:
    PressureSensor( const String& id, const Adafruit_BMP280& bmp );
    float getValue()  override;

  private:
    const Adafruit_BMP280& d_bmp;
};

// Current sensor - based on  ACS714  current sensor
class CurrentSensor : public Sensor {

  public:
    CurrentSensor( const String& id);
    void setup();
    float getValue()  override;
    void update(bool force_update=false);
    void resetAverages();
  private:
    float d_accumulation;
    int d_num_values = 0;
    float d_last_average = 0.0;
    int d_accum_cnt = 0;
};

// Battery Voltage sensor - reading off A0 pin which is attached to a 4-1 voltage divider
class BatteryVoltageSensor : public Sensor {

  public:
    BatteryVoltageSensor( const String& id);
    void setup();
    float getValue()  override;
    void update(bool force_update=false);
    void resetAverages();
  private:
    float d_accumulation;
    int d_num_values = 0;
    float d_last_average = 0.0;
    int d_accum_cnt = 0;
};



#endif
