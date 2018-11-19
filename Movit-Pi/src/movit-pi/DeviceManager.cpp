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
                                                         _alarm(10)
{
    _motionSensor = MotionSensor::GetInstance();
    _mobileImu = MobileImu::GetInstance();
    _fixedImu = FixedImu::GetInstance();
    _datetimeRTC = DateTimeRTC::GetInstance();
    _pressureMat = PressureMat::GetInstance();
}

void DeviceManager::InitializeDevices()
{
    I2Cdev::Initialize();

    _fileManager->Read();

    InitializePressureMat();
    InitializeMobileImu();
    InitializeFixedImu();

    _datetimeRTC->SetCurrentDateTimeThread().detach();
    _isAlarmInitialized = _alarm.Initialize();
    _isMotionSensorInitialized = _motionSensor->Initialize();

    _fileManager->Save();

    printf("Setup Done\n");
}

void DeviceManager::InitializePressureMat()
{
    pressure_mat_offset_t pressureMatOffset = _fileManager->GetPressureMatoffset();
    _pressureMat->SetOffsets(pressureMatOffset);
    _pressureMat->Initialize();
}
void DeviceManager::InitializeMobileImu()
{
    _isMobileImuInitialized = _mobileImu->Initialize();
    _isMobileImuCalibrated = false;

    if (!_isMobileImuInitialized)
    {
        return;
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
}

void DeviceManager::InitializeFixedImu()
{
    _isFixedImuInitialized = _fixedImu->Initialize();
    _isFixedImuCalibrated = false;

    if (!_isFixedImuInitialized)
    {
        return;
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
    default:
        throw "Error: Invalid device";
        break;
    }
}

void DeviceManager::ReconnectSensor(const int device)
{
    Sensor *sensor;
    try
    {
        sensor = GetSensor(device);
    }
    catch (const std::exception &e)
    {
        printf("Error: Invalid device\n");
        return;
    }

    if (!sensor->IsConnected())
    {
        if (typeid(sensor) == typeid(FixedImu))
        {
            InitializeFixedImu();
        }
        else if (typeid(sensor) == typeid(MobileImu))
        {
            InitializeMobileImu();
        }
        else if (typeid(sensor) == typeid(ForcePlate))
        {
            _pressureMat->Initialize();
        }
        else
        {
            sensor->Initialize();
        }
    }
}

bool DeviceManager::IsSensorStateChanged(const int device)
{
    Sensor *sensor;
    try
    {
        sensor = GetSensor(device);
    }
    catch (const std::exception &e)
    {
        printf("Error: Invalid device\n");
        return false;
    }

    if (sensor->IsStateChanged())
    {
        return true;
    }

    return false;
}

void DeviceManager::TurnOff()
{
    if (_isAlarmInitialized)
    {
        GetAlarm()->TurnOffAlarm();
    }
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
    if (_isMotionSensorInitialized)
    {
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

    if (IsPressureMatCalibrated())
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