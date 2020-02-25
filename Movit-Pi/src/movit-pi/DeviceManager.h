#ifndef DEVICE_MANAGER_H
#define DEVICE_MANAGER_H

#include <string>

#include "Imu.h"
#include "Alarm.h"

#include "FixedImu.h"
#include "MobileImu.h"
//#include "BackSeatAngleTracker.h"
#include "DateTimeRTC.h"
#include "PMW3901.h"
#include "VL53L0X.h"
#include "FileManager.h"
#include "Sensor.h"
#include "PressureMat.h"
#include "SeatingFSM.h"
#include "TravelFSM.h"
#include "AngleFSM.h"

class DeviceManager
{
  public:
    void InitializeDevices();

    // Called periodicaly to update all the data
    void Update();

    void CalibrateIMU();
    void CalibrateFixedIMU();
    void CalibrateMobileIMU();
    void CalibratePressureMat();

    SensorData getSensorData() {
      return sensorData; 
    }

    void TurnOff();

    void UpdateTiltSettings(tilt_settings_t tiltSettings);
    tilt_settings_t GetTiltSettings() { return _tiltSettings; }

    void UpdateNotificationsSettings(notifications_settings_t notificationsSettings);

    bool IsAlarmConnected();
    bool IsMobileImuConnected();
    bool IsFixedImuConnected();
    bool IsRangeSensorConnected();
    bool IsFlowSensorConnected();
    bool IsPressureMatConnected();
    bool IsUserSeated() { return _pressureMat->IsSomeoneThere(); }

    bool IsLedBlinkingEnabled() { return _notificationsSettings.isLedBlinkingEnabled; }
    bool IsVibrationEnabled() { return _notificationsSettings.isVibrationEnabled; }
    float GetSnoozeTime() { return _notificationsSettings.snoozeTime; }

    static DeviceManager *GetInstance(FileManager *fileManager)
    {
        static DeviceManager instance(fileManager);
        return &instance;
    }

    Alarm *getAlarm() {
      return &_alarm;
    }


  private:
    DeviceManager(FileManager *fileManager);

    const int32_t DEFAULT_BACK_SEAT_ANGLE = 0;

    bool InitializeFixedImu();
    bool InitializeMobileImu();
    bool InitializePressureMat();

    bool _isMoving = false;
    bool _isChairInclined = false;
    int _backSeatAngle = 0;
    int mobileFail = 0;
    int fixedFail = 0;
    long lastmIMUReset = 0;
    long lastfIMUReset = 0;

    FileManager *_fileManager;

    Alarm _alarm;
    MobileImu *_mobileImu;
    FixedImu *_fixedImu;
    PressureMat *_pressureMat;
    VL53L0X *_VL53L0XSensor;
    PMW3901 *_PMW3901Sensor;

    notifications_settings_t _notificationsSettings;
    tilt_settings_t _tiltSettings;

    SensorData sensorData = SensorData();
};

#endif // DEVICE_MANAGER_H
