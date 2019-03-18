#include "DeviceManager.h"
#include "NetworkManager.h"
#include "FixedImu.h"
#include "MobileImu.h"
#include "I2Cdev.h"
#include "Utils.h"
#include "SysTime.h"

#include <unistd.h>
#include <thread>


DeviceManager::DeviceManager(FileManager *fileManager) : _fileManager(fileManager),
                                                         _alarm(10) {
    _mobileImu = MobileImu::GetInstance();
    _fixedImu = FixedImu::GetInstance();
    _pressureMat = PressureMat::GetInstance();
    _VL53L0XSensor = new VL53L0X();
    _PMW3901Sensor = new PMW3901();
}

bool DeviceManager::IsAlarmConnected() {
    return sensorData.alarmConnected;
}

bool DeviceManager::IsMobileImuConnected() {
    return sensorData.mIMUConnected;
}

bool DeviceManager::IsFixedImuConnected() {
    return sensorData.mIMUConnected;
}

bool DeviceManager::IsRangeSensorConnected() {
    return _VL53L0XSensor->IsConnected();
}

bool DeviceManager::IsFlowSensorConnected() {
    return _PMW3901Sensor->IsConnected();
}

bool DeviceManager::IsPressureMatConnected() {
    return sensorData.matConnected;
}

void DeviceManager::InitializeDevices()
{
    I2Cdev::Initialize();

    _fileManager->Read();

    _notificationsSettings = _fileManager->GetNotificationsSettings();
    _tiltSettings = _fileManager->GetTiltSettings();
    InitializePressureMat();
    InitializeMobileImu();
    InitializeFixedImu();
    _alarm.Initialize();
    _PMW3901Sensor->Initialize();
    _VL53L0XSensor->Initialize();

    _fileManager->Save();
}

bool DeviceManager::InitializePressureMat() {
    pressure_mat_offset_t pressureMatOffset = _fileManager->GetPressureMatoffset();
    _pressureMat->SetOffsets(pressureMatOffset);
    _pressureMat->Initialize();
    sensorData.matConnected = _pressureMat->IsConnected();
    return sensorData.matConnected;
}

bool DeviceManager::InitializeMobileImu() {
    _mobileImu->Initialize();
    sensorData.mIMUConnected = _mobileImu->IsConnected();

    if (!sensorData.mIMUConnected)
    {
        return sensorData.mIMUConnected;
    }

    imu_offset_t mobileOffset = _fileManager->GetMobileImuOffsets();

    if (!Imu::IsImuOffsetValid(mobileOffset))
    {
        //CalibrateMobileIMU();
    }
    else
    {
        //_mobileImu->SetOffset(mobileOffset);
        sensorData.mIMUCalibrated = true;
    }

    return sensorData.mIMUConnected;
}

bool DeviceManager::InitializeFixedImu() {
    _fixedImu->Initialize();

    sensorData.fIMUConnected = _fixedImu->IsConnected();

    if (!sensorData.fIMUConnected)
    {
        return sensorData.fIMUConnected;
    }

    imu_offset_t fixedOffset = _fileManager->GetFixedImuOffsets();

    if (!Imu::IsImuOffsetValid(fixedOffset))
    {
        //CalibrateFixedIMU();
    }
    else
    {
        //_fixedImu->SetOffset(fixedOffset);
        sensorData.fIMUCalibrated = true;
    }

    return sensorData.fIMUConnected;
}

void DeviceManager::CalibratePressureMat() {
    printf("Calibrating pressure mat");
    _pressureMat->Calibrate();
    _fileManager->SetPressureMatOffsets(_pressureMat->GetOffsets());
    _fileManager->Save();
    printf("\n");
}

void DeviceManager::CalibrateIMU() {
    CalibrateFixedIMU();
    CalibrateMobileIMU();
}
 
void DeviceManager::CalibrateFixedIMU() {
    printf("Calibrating FixedIMU");
    _fixedImu->CalibrateAndSetOffsets();
    _fileManager->SetFixedImuOffsets(_fixedImu->GetOffset());
    _fileManager->Save();
    sensorData.fIMUCalibrated = true;
    printf("\n");
}

void DeviceManager::CalibrateMobileIMU() {
    printf("Calibrating MobileIMU");
    _mobileImu->CalibrateAndSetOffsets();
    _fileManager->SetMobileImuOffsets(_mobileImu->GetOffset());
    _fileManager->Save();
    sensorData.mIMUCalibrated = true;
    printf("\n");
}

void DeviceManager::Update() {
    double arr[3];
    //Reception données mobileIMU
    if(_mobileImu->IsConnected()) {
        _mobileImu->GetAccelerations(arr);
        sensorData.mIMUConnected = true;
        sensorData.mIMUAccX = arr[0];
        sensorData.mIMUAccY = arr[1];
        sensorData.mIMUAccZ = arr[2];
        
        _mobileImu->GetRotations(arr);
        sensorData.mIMUGyroX = arr[0];
        sensorData.mIMUGyroY = arr[1];
        sensorData.mIMUGyroZ = arr[2];
    } else {
        _mobileImu->Initialize();
        sensorData.mIMUConnected = false;
        sensorData.mIMUAccX = 0;
        sensorData.mIMUAccY = 0;
        sensorData.mIMUAccZ = 0;
        sensorData.mIMUGyroX = 0;
        sensorData.mIMUGyroY = 0;
        sensorData.mIMUGyroZ = 0;
    }

    if(_fixedImu->IsConnected()) {
        _fixedImu->GetAccelerations(arr);
        sensorData.fIMUConnected = true;
        sensorData.fIMUAccX = arr[0];
        sensorData.fIMUAccY = arr[1];
        sensorData.fIMUAccZ = arr[2];

        _fixedImu->GetRotations(arr);
        sensorData.fIMUGyroX = arr[0];
        sensorData.fIMUGyroY = arr[1];
        sensorData.fIMUGyroZ = arr[2];
    } else {
        _fixedImu->Initialize();
        sensorData.fIMUConnected = false;
        sensorData.fIMUAccX = 0;
        sensorData.fIMUAccY = 0;
        sensorData.fIMUAccZ = 0;
        sensorData.fIMUGyroX = 0;
        sensorData.fIMUGyroY = 0;
        sensorData.fIMUGyroZ = 0;
    }

    if(_VL53L0XSensor->IsConnected()) {
        sensorData.tofConnected = true;
        sensorData.tofRange = _VL53L0XSensor->ReadRangeSingleMillimeters();
    } else {
        sensorData.tofConnected = false;
        sensorData.tofRange = 450;                  //Default value to get a distance when no tof
        _VL53L0XSensor->Initialize(true);
    }

    if(_PMW3901Sensor->IsConnected()) {
        sensorData.flowConnected = true;
        _PMW3901Sensor->ReadMotionCount(&sensorData.flowTravelX, &sensorData.flowTravelY);
    } else {
        sensorData.flowConnected = false;
        sensorData.flowTravelX = 0;
        sensorData.flowTravelY = 0;
        _PMW3901Sensor->Initialize();
    }

    if(_alarm.IsConnected()) {
        sensorData.alarmConnected = true;
        sensorData.alarmButtonPressed = _alarm.ButtonPressed();
    } else {
        sensorData.alarmConnected = false;
        sensorData.alarmRedLedOn = false;
        sensorData.alarmGreenLedOn = false;
        sensorData.alarmDCMotorOn = false;
        sensorData.alarmButtonPressed = false;
        _alarm.Initialize();
    }

    if(_pressureMat->IsConnected()) {
        sensorData.matConnected = true;
        _pressureMat->Update();
        _pressureMat->GetRawAnalogData(sensorData.matData);
    } else {
        sensorData.matConnected = false;
        _pressureMat->Initialize();
    }

    sensorData.time = DateTimeRTC::GetTimeSinceEpoch();
}

void DeviceManager::UpdateNotificationsSettings(notifications_settings_t notificationsSettings) {
    _notificationsSettings = notificationsSettings;
    _fileManager->SetNotificationsSettings(notificationsSettings);
    _fileManager->Save();
}

void DeviceManager::TurnOff() {
    if (sensorData.alarmConnected)
    {
        _alarm.TurnOffAlarm();
    }
}

void DeviceManager::UpdateTiltSettings(tilt_settings_t tiltSettings) {
    _tiltSettings = tiltSettings;
    _fileManager->SetTiltSettings(tiltSettings);
    _fileManager->Save();
}
