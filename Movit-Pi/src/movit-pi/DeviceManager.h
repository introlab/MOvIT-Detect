#ifndef DEVICE_MANAGER_H
#define DEVICE_MANAGER_H

#include <string>

#include "Imu.h"
#include "Alarm.h"
#include "ForcePlate.h"
#include "ForceSensor.h"
#include "FixedImu.h"
#include "MobileImu.h"
#include "BackSeatAngleTracker.h"
#include "DateTimeRTC.h"
#include "MotionSensor.h"
#include "MAX11611.h"
#include "FileManager.h"
#include "Sensor.h"

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
    MobileImu *GetMobileImu() { return _mobileImu; }
    FixedImu *GetFixedImu() { return _fixedImu; }
    MotionSensor *GetMotionSensor() { return _motionSensor; }

    bool IsForcePlateConnected();
    void ReconnectSensor(const int device);
    bool IsSensorStateChanged(const int device);

    // TODO: CECI EST TEMPORAIRE CAR ON A PU DE TAPIS DE PRESSION
    //bool IsSomeoneThere() { return _isSomeoneThere; }
    bool IsMoving() { return _isMoving; }
    bool IsSomeoneThere() { return true; }
    bool IsChairInclined() { return _isChairInclined; }

    Coord_t GetCenterOfPressure() { return _COPCoord; }
    int GetBackSeatAngle() { return _backSeatAngle; }
    int GetTimeSinceEpoch() { return _timeSinceEpoch; }
    double GetXAcceleration();

    void CalibrateIMU();
    void CalibrateFixedIMU();
    void CalibrateMobileIMU();
    void CalibratePressureMat();

    void TurnOff();

    bool TestDevices();

    bool IsAlarmConnected() { return _alarm.IsConnected(); }
    bool IsMobileImuConnected() { return _mobileImu->IsConnected(); }
    bool IsFixedImuConnected() { return _fixedImu->IsConnected(); }
    bool IsMotionSensorConnected() { return _motionSensor->IsConnected(); }

    bool IsImuCalibrated() { return _isFixedImuCalibrated && _isMobileImuCalibrated; }
    bool IsPressureMatCalibrated() { return _isPressureMatCalibrated; }

    //To be validated, we don't use them anywhere.
    //bool GetIsAlarmInitialized() { return _isAlarmInitialized; }
    //bool GetIsFixedImuInitialized() { return _isFixedImuInitialized; }
    //bool GetIsMobileInitialized() { return _isMobileImuInitialized; }
    //bool GetIsMotionSensorInitialized() { return _isMotionSensorInitialized; }
    //bool GetIsForcePlateInitialized() { return _isForcePlateInitialized; }

    // Singleton
    static DeviceManager *GetInstance(FileManager *fileManager)
    {
        static DeviceManager instance(fileManager);
        return &instance;
    }

  private:
    //Singleton
    DeviceManager(FileManager *fileManager);
    DeviceManager(DeviceManager const &);  // Don't Implement.
    void operator=(DeviceManager const &); // Don't implement.

    const int32_t DEFAULT_BACK_SEAT_ANGLE = 0;
    const float DEFAULT_CENTER_OF_PRESSURE = 0.0;

    Sensor *GetSensor(const int device);
    void UpdateForcePlateData();
    void InitializeForcePlateSensors();
    void InitializeFixedImu();
    void InitializeMobileImu();
    bool InitializeForcePlate();

    bool _isAlarmInitialized = false;
    bool _isFixedImuInitialized = false;
    bool _isMobileImuInitialized = false;
    bool _isMotionSensorInitialized = false;
    bool _isForcePlateInitialized = false;

    bool _isFixedImuCalibrated = false;
    bool _isMobileImuCalibrated = false;
    bool _isPressureMatCalibrated = false;

    bool _isMoving = false;
    bool _isSomeoneThere = false;
    bool _isChairInclined = false;

    Coord_t _COPCoord;
    int _timeSinceEpoch = 0;
    int _backSeatAngle = 0;
    uint16_t _max11611Data[9];

    FileManager *_fileManager;

    DateTimeRTC *_datetimeRTC;
    Alarm _alarm;
    MobileImu *_mobileImu;
    FixedImu *_fixedImu;
    BackSeatAngleTracker _backSeatAngleTracker;
    ForceSensor _sensorMatrix;
    ForcePlate _globalForcePlate;
    MAX11611 _max11611;
    MotionSensor *_motionSensor;
};

#endif // DEVICE_MANAGER_H
