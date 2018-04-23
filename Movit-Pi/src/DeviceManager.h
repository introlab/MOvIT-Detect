#ifndef DEVICE_MANAGER_H
#define DEVICE_MANAGER_H

#include "Sensors/Alarm.h"
#include "Sensors/ForcePlate.h"
#include "Sensors/ForceSensor.h"
#include "Sensors/BackSeatAngleTracker.h"
#include "Sensors/DateTimeRTC.h"
#include "Sensors/MotionSensor.h"
#include "MAX11611.h"
#include <string>

//Center of pressure coordinate
struct Coord_t
{
    float x;
    float y;
};

class DeviceManager
{
  public:
    void InitializeDevices();

    // Called periodicaly to update all the data
    void Update();

    Alarm *GetAlarm() { return &_alarm; }

    bool IsSomeoneThere() { return _isSomeoneThere; }
    Coord_t GetCenterOfPressure() { return _COPCoord; }
    int GetBackSeatAngle() { return _backSeatAngle; }
    int GetTimeSinceEpoch() { return _timeSinceEpoch; }
    bool GetIsMoving() { return _isMoving; }
    void CalibratePressureMat();
    void CalibrateIMU();
    void TurnOff();

    bool TestDevices();

    // Singleton
    static DeviceManager *GetInstance()
    {
        static DeviceManager instance;
        return &instance;
    }

  private:
    //Singleton
    DeviceManager();
    DeviceManager(DeviceManager const &);  // Don't Implement.
    void operator=(DeviceManager const &); // Don't implement.

    void UpdateForcePlateData();
    bool InitializeForcePlate();

    bool _imuValid = false;
    bool _forcePlateValid = false;
    bool _isMoving = false;
    bool _isSomeoneThere = false;

    Coord_t _COPCoord;
    int _timeSinceEpoch = 0;

    DateTimeRTC *_datetimeRTC;
    Alarm _alarm;
    BackSeatAngleTracker _imu;
    int _backSeatAngle = 0;

    uint16_t _max11611Data[9];
    ForceSensor _sensorMatrix;
    ForcePlate _globalForcePlate;
    MotionSensor _motionSensor;
    MAX11611 _max11611;
};

#endif // DEVICE_MANAGER_H
