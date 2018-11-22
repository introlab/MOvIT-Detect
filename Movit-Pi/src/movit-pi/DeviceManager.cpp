#include "DeviceManager.h"
#include "NetworkManager.h"
#include "FixedImu.h"
#include "MobileImu.h"
#include "I2Cdev.h"
#include "Utils.h"
#include "SysTime.h"

#include <unistd.h>
#include <thread>

class InvalidSensorException : public std::exception
{
  public:
    InvalidSensorException(const int device) { _device = device; }
    virtual const char *what() const throw()
    {
        std::string message = "Error: Invalid device. Device given: " + std::to_string(_device) + "\n";
        return message.c_str();
    }

  private:
    int _device = 0;
};

DeviceManager::DeviceManager(FileManager *fileManager) : _fileManager(fileManager),
                                                         _alarm(10)
{
    _motionSensor = MotionSensor::GetInstance();
    _mobileImu = MobileImu::GetInstance();
    _fixedImu = FixedImu::GetInstance();
    _datetimeRTC = DateTimeRTC::GetInstance();
    _pressureMat = PressureMat::GetInstance();
}

bool DeviceManager::IsAlarmConnected()
{
    _isAlarmInitialized = _alarm.IsConnected();
    return _isAlarmInitialized;
}

bool DeviceManager::IsMobileImuConnected()
{
    _isMobileImuInitialized = _mobileImu->IsConnected();
    return _isMobileImuInitialized;
}

bool DeviceManager::IsFixedImuConnected()
{
    _isFixedImuInitialized = _fixedImu->IsConnected();
    return _isFixedImuInitialized;
}

bool DeviceManager::IsMotionSensorConnected()
{
    _isMotionSensorInitialized = _motionSensor->IsConnected();
    return _isMotionSensorInitialized;
}

bool DeviceManager::IsPressureMatConnected()
{
    _isPressureMatInitialized = _pressureMat->IsConnected();
    return _isPressureMatInitialized;
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

    _isAlarmInitialized = _alarm.Initialize();
    _isMotionSensorInitialized = _motionSensor->Initialize();
    _datetimeRTC->SetCurrentDateTimeThread().detach();

    _fileManager->Save();

    printf("Setup Done\n");
}

bool DeviceManager::InitializePressureMat()
{
    pressure_mat_offset_t pressureMatOffset = _fileManager->GetPressureMatoffset();
    _pressureMat->SetOffsets(pressureMatOffset);
    _isPressureMatInitialized = _pressureMat->Initialize();
    return _isPressureMatInitialized;
}

bool DeviceManager::InitializeMobileImu()
{
    _isMobileImuInitialized = _mobileImu->Initialize();
    _isMobileImuCalibrated = false;

    if (!_isMobileImuInitialized)
    {
        return _isMobileImuInitialized;
    }

    imu_offset_t mobileOffset = _fileManager->GetMobileImuOffsets();

    if (!Imu::IsImuOffsetValid(mobileOffset))
    {
        CalibrateMobileIMU();
    }
    else
    {
        _mobileImu->SetOffset(mobileOffset);
        _isMobileImuCalibrated = true;
    }

    return _isMobileImuInitialized;
}

bool DeviceManager::InitializeFixedImu()
{
    _isFixedImuInitialized = _fixedImu->Initialize();
    _isFixedImuCalibrated = false;

    if (!_isFixedImuInitialized)
    {
        return _isFixedImuInitialized;
    }

    imu_offset_t fixedOffset = _fileManager->GetFixedImuOffsets();

    if (!Imu::IsImuOffsetValid(fixedOffset))
    {
        CalibrateFixedIMU();
    }
    else
    {
        _fixedImu->SetOffset(fixedOffset);
        _isFixedImuCalibrated = true;
    }

    return _isFixedImuInitialized;
}

void DeviceManager::CalibratePressureMat()
{
    printf("Calibrating pressure mat ... \n");
    _pressureMat->Calibrate();
    _fileManager->SetPressureMatOffsets(_pressureMat->GetOffsets());
    _fileManager->Save();
    printf("DONE\n");
}

void DeviceManager::CalibrateIMU()
{
    CalibrateFixedIMU();
    CalibrateMobileIMU();
}

void DeviceManager::CalibrateFixedIMU()
{
    printf("Calibrating FixedIMU ... \n");
    _fixedImu->CalibrateAndSetOffsets();
    _fileManager->SetFixedImuOffsets(_fixedImu->GetOffset());
    _fileManager->Save();
    _isFixedImuCalibrated = true;
    printf("DONE\n");
}

void DeviceManager::CalibrateMobileIMU()
{
    printf("Calibrating MobileIMU ... \n");
    _mobileImu->CalibrateAndSetOffsets();
    _fileManager->SetMobileImuOffsets(_mobileImu->GetOffset());
    _fileManager->Save();
    _isMobileImuCalibrated = true;
    printf("DONE\n");
}

void DeviceManager::Update()
{
    _timeSinceEpoch = _datetimeRTC->GetTimeSinceEpoch();

    _sensorState.notificationModuleValid = GetSensorValidity(DEVICES::alarmSensor, IsAlarmConnected());
    _sensorState.fixedAccelerometerValid = GetSensorValidity(DEVICES::fixedImu, IsFixedImuConnected());
    _sensorState.mobileAccelerometerValid = GetSensorValidity(DEVICES::mobileImu, IsMobileImuConnected());
    _sensorState.pressureMatValid = GetSensorValidity(DEVICES::pressureMat, IsPressureMatConnected());
    // TODO: Refactorer le test de connection du motion sensor car il prend beaucoup trop de temps
    // UpdateSensor(DEVICES::motionSensor, _deviceManager->IsMotionSensorConnected());

    if (_isMotionSensorInitialized)
    {
        _motionSensor->GetDeltaXY();
        _isMoving = _motionSensor->IsMoving();
    }

    if (_isFixedImuInitialized && _isMobileImuInitialized && _isFixedImuCalibrated && _isMobileImuCalibrated)
    {
        // Data: Angle (centrales intertielles mobile/fixe)
        _backSeatAngle = _backSeatAngleTracker.GetBackSeatAngle();
    }
    else
    {
        _backSeatAngle = DEFAULT_BACK_SEAT_ANGLE;
    }

    if (_isFixedImuInitialized && _isFixedImuCalibrated)
    {
        _isChairInclined = _backSeatAngleTracker.IsInclined();
    }

    if (_isPressureMatInitialized && IsPressureMatCalibrated())
    {
        _pressureMat->Update();
    }
}

double DeviceManager::GetXAcceleration()
{
    if (_isFixedImuInitialized)
    {
        return _fixedImu->GetXAcceleration();
    }
    return 0;
}

void DeviceManager::UpdateNotificationsSettings(notifications_settings_t notificationsSettings)
{
    _fileManager->SetNotificationsSettings(notificationsSettings);
    _fileManager->Save();
}

Sensor *DeviceManager::GetSensor(const int device)
{
    switch (device)
    {
    case alarmSensor:
        return &_alarm;
    case mobileImu:
        return _mobileImu;
    case fixedImu:
        return _fixedImu;
    case motionSensor:
        return _motionSensor;
    case pressureMat:
        return _pressureMat;
    default:
        throw InvalidSensorException(device);
        break;
    }
}

bool DeviceManager::IsSensorStateChanged(const int device)
{
    Sensor *sensor;
    try
    {
        sensor = GetSensor(device);
    }
    catch (const InvalidSensorException &e)
    {
        printf("%s", e.what());
        return false;
    }

    if (sensor->IsStateChanged())
    {
        return true;
    }

    return false;
}

bool DeviceManager::GetSensorValidity(const int device, const bool isConnected)
{
    bool ret = isConnected;
    Sensor *sensor;
    try
    {
        sensor = GetSensor(device);
    }
    catch (const InvalidSensorException &e)
    {
        printf("%s", e.what());
        return false;
    }

    if (!isConnected)
    {
        if (typeid(sensor) == typeid(FixedImu))
        {
            ret = InitializeFixedImu();
        }
        else if (typeid(sensor) == typeid(MobileImu))
        {
            ret = InitializeMobileImu();
        }
        else if (typeid(sensor) == typeid(ForcePlate))
        {
            ret = InitializePressureMat();
        }
        else
        {
            ret = sensor->Initialize();
        }
    }

    return ret;
}

void DeviceManager::TurnOff()
{
    if (_isAlarmInitialized)
    {
        GetAlarm()->TurnOffAlarm();
    }
}

void DeviceManager::UpdateTiltSettings(tilt_settings_t tiltSettings)
{
    _tiltSettings = tiltSettings;
    _fileManager->SetTiltSettings(tiltSettings);
    _fileManager->Save();
}
