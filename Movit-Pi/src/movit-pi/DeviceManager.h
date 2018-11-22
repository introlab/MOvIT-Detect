#ifndef DEVICE_MANAGER_H
#define DEVICE_MANAGER_H

#include <string>

#include "Imu.h"
#include "Alarm.h"

#include "FixedImu.h"
#include "MobileImu.h"
#include "BackSeatAngleTracker.h"
#include "DateTimeRTC.h"
#include "MotionSensor.h"
#include "FileManager.h"
#include "Sensor.h"
#include "PressureMat.h"

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

    bool IsSomeoneThere() { return _pressureMat->IsSomeoneThere(); }
    bool IsChairInclined() { return _isChairInclined; }
    bool IsForcePlateConnected() { return _pressureMat->IsConnected(); }
    bool IsMoving() { return _isMoving; }

    pressure_mat_data_t GetPressureMatData() { return _pressureMat->GetPressureMatData(); }

    int GetBackSeatAngle() { return _backSeatAngle; }
    int GetTimeSinceEpoch() { return _timeSinceEpoch; }

    double GetXAcceleration();

    void CalibrateIMU();
    void CalibrateFixedIMU();
    void CalibrateMobileIMU();
    void CalibratePressureMat();

    void TurnOff();

    void UpdateTiltSettings(tilt_settings_t tiltSettings);
    tilt_settings_t GetTiltSettings() { return _tiltSettings; }

    void UpdateNotificationsSettings(notifications_settings_t notificationsSettings);

    bool IsAlarmConnected();
    bool IsMobileImuConnected();
    bool IsFixedImuConnected();
    bool IsMotionSensorConnected();
    bool IsPressureMatConnected();

    bool IsImuCalibrated() { return _isFixedImuCalibrated && _isMobileImuCalibrated; }
    bool IsPressureMatCalibrated() { return _pressureMat->IsCalibrated(); }

    bool IsLedBlinkingEnabled() { return _notificationsSettings.isLedBlinkingEnabled; }
    bool IsVibrationEnabled() { return _notificationsSettings.isVibrationEnabled; }

    sensor_state_t GetSensorState() { return _sensorState; }

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

    Sensor *GetSensor(const int device);
    bool GetSensorValidity(const int device, const bool isConnected);
    bool IsSensorStateChanged(const int device);

    bool InitializeFixedImu();
    bool InitializeMobileImu();
    bool InitializePressureMat();

    bool _isAlarmInitialized = false;
    bool _isFixedImuInitialized = false;
    bool _isMobileImuInitialized = false;
    bool _isMotionSensorInitialized = false;
    bool _isPressureMatInitialized = false;

    bool _isFixedImuCalibrated = false;
    bool _isMobileImuCalibrated = false;

    bool _isMoving = false;
    bool _isChairInclined = false;

    int _timeSinceEpoch = 0;
    int _backSeatAngle = 0;

    FileManager *_fileManager;

    DateTimeRTC *_datetimeRTC;
    Alarm _alarm;
    MobileImu *_mobileImu;
    FixedImu *_fixedImu;
    BackSeatAngleTracker _backSeatAngleTracker;
    PressureMat *_pressureMat;
    MotionSensor *_motionSensor;

    notifications_settings_t _notificationsSettings;
    sensor_state_t _sensorState;
    tilt_settings_t _tiltSettings;
};

#endif // DEVICE_MANAGER_H
