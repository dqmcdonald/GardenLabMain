/* Declarations for all sensor classes */

class Sensor {

  public:
    Sensor(const String& id);                   // Initialize with four letter ID used to send
                                                // data

    virtual void  setup();                      // Setup state of pins etc
    virtual float getValue() const = 0;         // Get the current value
    virtual void  update();                     // Do any required periodic updating tasks
    virtual void  sendData(Stream& serial);     // Send data over specified serial Stream


  protected:
    const String d_id;

};



