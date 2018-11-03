#ifndef SENSOR_H
#define SENSOR_H

class Sensor
{
  public:
    virtual bool Initialize() = 0;
    virtual bool IsConnected() = 0;
    bool IsStateChanged();

  protected:
    bool _isConnected = false;
};

#endif // SENSOR_H
