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
    Sensor(const String& id);                  // Initialize with four letter ID used to send data
    virtual ~Sensor();
    virtual void  setup();                     // Setup state of pins etc
    virtual float getValue()  = 0;             // Get the current value
    virtual void  update(bool force_update = false);// Do any required periodic updating tasks

    virtual String  getDataString();           // Return a string in the form ID=VALUE, suitable for sending to the server
    virtual void resetAverages();              // Reset any average values being accumulated
    virtual void resetAccumulation();          // For accumulating sensors - reset their values

  protected:
    const String d_id;

};


// Averaging sensor does a read many times during the loop() and returns an average value. This is typically reset each time
// the LCD display is updated.
class AveragingSensor : public Sensor {

  public:
    AveragingSensor(const String& id);                  // Initialize with four letter ID used to send data
    virtual ~AveragingSensor();

    virtual void  setup();                     // Setup state of pins etc
    virtual float getValue() ;                // Get the current value
    virtual void  update(bool force_update = false);// Do any required periodic updating tasks
    virtual void resetAverages();              // Reset any average values being accumulated

  protected:

    virtual float readSensor() = 0;

  protected:
    float d_accumulation;
    int d_num_values = 0;
    float d_last_average = 0.0;
    int d_accum_cnt = 0;
};



// Temperature sensor, Based on DHT sensor

class TemperatureSensor : public Sensor {

  public:
    TemperatureSensor( const String& id,  DHT& dht );

    float getValue()  override;

  private:
     DHT& d_dht;
};


// Humidity sensor, Based on DHT sensor
class HumiditySensor : public Sensor {

  public:
    HumiditySensor( const String& id,  DHT& dht );

    float getValue()  override;

  private:
     DHT& d_dht;
};


// Temperature sensor, Based on BMP280 sensor
class InsideTempSensor : public Sensor {

  public:
    InsideTempSensor( const String& id,  Adafruit_BMP280& bmp );

    float getValue()  override;

  private:
     Adafruit_BMP280& d_bmp;
};


// Pressure sensor, Based on BMP280 sensor
class PressureSensor : public Sensor {

  public:
    PressureSensor( const String& id,  Adafruit_BMP280& bmp );
    ~PressureSensor() { };
    float getValue()  override;

  private:
     Adafruit_BMP280& d_bmp;
};

// Current sensor - based on  ACS714  current sensor
class CurrentSensor : public AveragingSensor {

  public:
    CurrentSensor( const String& id);
    ~CurrentSensor() {};
    void setup();

  protected:
    virtual float readSensor() override;

  private:

};

// Battery Voltage sensor - reading off A0 pin which is attached to a 4-1 voltage divider
class BatteryVoltageSensor : public AveragingSensor {

  public:
    BatteryVoltageSensor( const String& id);

    void setup() override;
  protected:
    virtual float readSensor() override;
  private:

};


// Wind speed sensor - works on interrupt on pin D2
class WindSpeedSensor : public Sensor {
  public:
    WindSpeedSensor( const String& id, const int& pin );

    void setup() override;
    void update(bool) override;
    float getValue()  override;
    static void windSpeedUpdate();
  private:
    int d_pin;
    unsigned long d_update_time = 0;
    float d_wind_speed_value = 0;
};

// Wind dirction sensor - reads voltage off A2 uses table to convert to
class WindDirectionSensor : public AveragingSensor {
  public:
    WindDirectionSensor( const String& id, const int& pin );

    void setup() override;

  protected:
    virtual float readSensor() override;
  private:
    float voltageToDegrees(float voltage );

    int d_pin;

};

// Rainfall sensor. Tipping bucket switch connected to pin D3
class RainfallSensor : public Sensor {
  public:
    RainfallSensor( const String& id, const int& pin );
    void setup() override;
    void resetAccumulation()override;
    void update(bool) override;
     float getValue()  override;

     
    static void rainfallUpdate();

  private:
    float d_accumulated_rainfall = 0.0;
int d_pin;
unsigned long int d_update_time;
};



#endif
