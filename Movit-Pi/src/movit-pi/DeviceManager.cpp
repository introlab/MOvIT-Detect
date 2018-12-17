#include "DeviceManager.h"
#include "NetworkManager.h"
#include "FixedImu.h"
#include "MobileImu.h"
#include "I2Cdev.h"
#include "Utils.h"
#include "MAX11611.h"
#include "ForceSensor.h"
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
    _notificationsSettings = notificationsSettings;
    _fileManager->SetNotificationsSettings(notificationsSettings);
    _fileManager->Save();
}

bool DeviceManager::TestDevices()
{
    printf("\n.-.--..--- MAIN MODULES TEST MENU .--.---.--.-\n");
    printf("\n ID \t Module Description");
    printf("\n--------------------------------------");
    printf("\n 1 \t Module Header Raspberry Pi");
    printf("\n 2 \t Module Notification");
    printf("\n 3 \t Module IMU");
    printf("\n 4 \t Module Pressure Mat");
    printf("\n 5 \t Module Flow Sensor");
    printf("\n q \t *Close Main Test Menu");
    printf("\n--------------------------------------");
    printf("\n Enter a ModuleID and press ENTER to see module tests\n");

    char moduleID = getchar();
    char testNoID = 99; // out of range value
    int loopTest = 0;
    int subTestExit = 0;
    getchar(); // To consume '\n'

    switch (moduleID)
    {
    case '1':
    {
        subTestExit = 0;
        while (subTestExit != 1)
        {
            printf("\nHEADER RASPBERRY PI MODULE\n");

            printf("\n.-.--..--- HEADER RASPBERRY PI TEST MENU .--.---.--.-\n");
            printf("\nTestID\tDescription");
            printf("\n--------------------------------------");
            printf("\n 1 \t RTC date and time validation");
            printf("\n q \t *Back to Main Test Menu");
            printf("\n--------------------------------------");
            printf("\nEnter a Test No. and press the return key to run test\n");
            testNoID = getchar();
            getchar(); // To consume '\n'
            loopTest = 0;

            switch (testNoID)
            {
            case '1':
            {
                printf("\n.-.--..---.-.-.--.--.--.---.--.-\n");
                printf("TEST NO. : %c\n", testNoID);
                printf("RTC date and time validation\n");
                while (loopTest != 27)
                {
                    time_t currentTimeRTC = _datetimeRTC->GetRTCCurrentTime();
                    time_t currentTimeSys = _datetimeRTC->GetSystemCurrentTime();
                    struct tm datetimeRTC;
                    struct tm datetimeSys;
                    char buf[80];
                    // Format time, "ddd yyyy-mm-dd hh:mm:ss zzz"
                    datetimeRTC = *localtime(&currentTimeRTC);
                    datetimeSys = *localtime(&currentTimeSys);
                    strftime(buf, sizeof(buf), "%a %Y-%m-%d\n%H:%M:%S %Z", &datetimeRTC);
                    printf("\nRTC CURRENT DATE AND TIME\n%s\n", buf);
                    strftime(buf, sizeof(buf), "%a %Y-%m-%d\n%H:%M:%S %Z", &datetimeSys);
                    printf("\nSystem CURRENT DATE AND TIME\n%s\n", buf);

                    printf("\nENTER to restart test sequence\n");
                    printf("ESC+ENTER to exit test sequence\n");
                    printf(".-.--..---.-.-.--.--.--.---.--.-\n");
                    loopTest = getchar();
                }
                getchar();
                break;
            }
            case 'q':
            {
                subTestExit = 1;
                break;
            }
            default:
            {
                printf("\nInvalid TestID = %i\n", testNoID);
            }
            break;
            }
        }
        break;
    }
    case '2':
    {
        subTestExit = 0;
        while (subTestExit != 1)
        {
            printf("\nNOTIFICATION MODULE\n");

            printf("\n.-.--..--- NOTIFICATION TEST MENU .--.---.--.-\n");
            printf("\nTestID\tDescription");
            printf("\n--------------------------------------");
            printf("\n 1 \t LEDs RG validation");
            printf("\n 2 \t DC motor - buzzer validation");
            printf("\n 3 \t Red blink alarm validation");
            printf("\n 4 \t Push-button validation");
            printf("\n q \t *Back to Main Test Menu");
            printf("\n--------------------------------------");
            printf("\nEnter a Test No. and press the return key to run test\n");
            testNoID = getchar();
            getchar(); // To consume '\n'
            loopTest = 0;

            switch (testNoID)
            {
            case '1':
            {
                printf("\n.-.--..---.-.-.--.--.--.---.--.-\n");
                printf("TEST NO. : %c\n", testNoID);
                printf("LEDs RG validation\n");
                while (loopTest != 27)
                {
                    printf("\nGREEN LED ON - ENTER for next test");
                    _alarm.TurnOnGreenLed();
                    _alarm.TurnOffRedLed();
                    getchar();

                    printf("RED LED ON - ENTER for next test");
                    _alarm.TurnOnRedLed();
                    _alarm.TurnOffGreenLed();
                    getchar();

                    printf("BLINK GREEN-RED - ENTER to power off LEDs");
                    _alarm.TurnOnBlinkLedsAlarmThread().detach();
                    getchar();
                    _alarm.StopBlinkLedsAlarm();

                    printf("\nENTER to restart test sequence\n");
                    printf("ESC+ENTER to exit test sequence\n");
                    printf(".-.--..---.-.-.--.--.--.---.--.-\n");
                    loopTest = getchar();
                }
                getchar();
                break;
            }
            case '2':
            {
                printf("\n.-.--..---.-.-.--.--.--.---.--.-\n");
                printf("TEST NO. : %c\n", testNoID);
                printf("DC motor validation\n");
                while (loopTest != 27)
                {
                    printf("\nDC MOTOR/BUZZER ON - ENTER to power off DC motor");
                    _alarm.TurnOnDCMotor();
                    getchar();
                    _alarm.TurnOffDCMotor();

                    printf("\nENTER to repeat test\n");
                    printf("ESC+ENTER to exit test\n");
                    printf(".-.--..---.-.-.--.--.--.---.--.-\n");
                    loopTest = getchar();
                }
                getchar();
                break;
            }
            case '3':
            {
                printf("\n.-.--..---.-.-.--.--.--.---.--.-\n");
                printf("TEST NO. : %c\n", testNoID);
                printf("Red blink alarm validation\n\n");
                while (loopTest != 27)
                {
                    printf("RED ALARM ON (3 sec.) - ENTER to stop alarm\n");
                    _alarm.TurnOnBlinkRedAlarmThread().detach();
                    uint8_t alarmTime = 3;
                    for (uint8_t i = 0; i < alarmTime; i++)
                    {
                        printf("%i\n", (alarmTime - i));
                        sleep_for_milliseconds(1000);
                    }
                    _alarm.StopBlinkRedAlarm();

                    printf("\nENTER to repeat test\n");
                    printf("ESC+ENTER to exit test\n");
                    printf(".-.--..---.-.-.--.--.--.---.--.-\n");
                    loopTest = getchar();
                }
                getchar();
                break;
            }
            case '4':
            {
                printf("\n.-.--..---.-.-.--.--.--.---.--.-\n");
                printf("TEST NO. : %c\n", testNoID);
                printf("Push-button validation\n");
                while (loopTest != 27)
                {
                    printf("\nPUSH BUTTON STATE\n");
                    for (uint8_t i = 0; i < 5; i++)
                    {
                        if (_alarm.ButtonPressed())
                        {
                            printf("Reading %i : PUSH-BUTTON PRESSED\n", (i + 1));
                        }
                        else
                        {
                            printf("Reading %i : ... \n", (i + 1));
                        }
                        sleep_for_milliseconds(1000);
                    }
                    printf("\nENTER to repeat test\n");
                    printf("ESC+ENTER to exit test\n");
                    printf(".-.--..---.-.-.--.--.--.---.--.-\n");
                    loopTest = getchar();
                }
                getchar();
                break;
            }
            case 'q':
            {
                subTestExit = 1;
                break;
            }
            default:
            {
                printf("\nInvalid TestID = %i\n", testNoID);
            }
            break;
            }
        }
        break;
    }
    case '3':
    {
        subTestExit = 0;
        while (subTestExit != 1)
        {
            printf("\nMODULE IMUs\n");

            printf("\n.-.--..--- IMUs TEST MENU .--.---.--.-\n");
            printf("\nTestID\tDescription");
            printf("\n--------------------------------------");
            printf("\n 1 \t IMU I2C Address validation");
            printf("\n 2 \t IMU Calibration validation");
            printf("\n 3 \t IMU Angles validation");
            printf("\n q \t *Back to Main Test Menu");
            printf("\n--------------------------------------");
            printf("\nEnter a Test No. and press the return key to run test\n");
            testNoID = getchar();
            getchar(); // To consume '\n'
            loopTest = 0;

            switch (testNoID)
            {
            case '1':
            {
                printf("\n.-.--..---.-.-.--.--.--.---.--.-\n");
                printf("TEST NO. : %c\n", testNoID);
                printf("IMU I2C Address validation\n");
                while (loopTest != 27)
                {
                    //asdahsd

                    printf("\nENTER to restart test sequence\n");
                    printf("ESC+ENTER to exit test sequence\n");
                    printf(".-.--..---.-.-.--.--.--.---.--.-\n");
                    loopTest = getchar();
                }
                getchar();
                break;
            }
            case '2':
            {
                printf("\n.-.--..---.-.-.--.--.--.---.--.-\n");
                printf("TEST NO. : %c\n", testNoID);
                printf("IMU Calibration validation\n");
                while (loopTest != 27)
                {
                    CalibrateIMU();

                    printf("\nENTER to restart test sequence\n");
                    printf("ESC+ENTER to exit test sequence\n");
                    printf(".-.--..---.-.-.--.--.--.---.--.-\n");
                    loopTest = getchar();
                }
                getchar();
                break;
            }
            case '3':
            {
                printf("\n.-.--..---.-.-.--.--.--.---.--.-\n");
                printf("TEST NO. : %c\n", testNoID);
                printf("IMU Angles validation\n");
                while (loopTest != 27)
                {
                    for (int i = 0; i < 8; i++)
                    {
                        int angleTest = 15 * i;
                        printf("\nReading %i : INCLINE mobileIMU AT %i deg\n", (i + 1), angleTest);
                        printf("ENTER when IMU is at desired angle");
                        getchar();
                        _backSeatAngle = _backSeatAngleTracker.GetBackSeatAngle();
                        printf("IMU measured angle : %i deg\n\n", _backSeatAngle);
                    }
                    printf("\nENTER to restart test sequence\n");
                    printf("ESC+ENTER to exit test sequence\n");
                    printf(".-.--..---.-.-.--.--.--.---.--.-\n");
                    loopTest = getchar();
                }
                getchar();
                break;
            }
            case 'q':
            {
                subTestExit = 1;
                break;
            }
            default:
            {
                printf("\nInvalid TestID = %i\n", moduleID);
            }
            break;
            }
        }
        break;
    }
    case '4':
    {
        ForceSensor forceSensor;
        subTestExit = 0;
        while (subTestExit != 1)
        {
            printf("\nPRESSURE MAT MODULE\n");

            printf("\n.-.--..--- PRESSURE MAT TEST MENU .--.---.--.-\n");
            printf("\nTestID\tDescription");
            printf("\n--------------------------------------");
            printf("\n 1 \t Force sensors (ADC) validation");
            printf("\n 2 \t Force sensors calibration validation");
            printf("\n 3 \t Presence detection validation");
            printf("\n 4 \t Force plates validation");
            printf("\n 5 \t Center of pressure validation");
            printf("\n q \t *Back to Main Test Menu");
            printf("\n--------------------------------------");
            printf("\nEnter a Test No. and press the return key to run test\n");
            testNoID = getchar();
            getchar(); // To consume '\n'
            loopTest = 0;

            switch (testNoID)
            {
            case '1':
            {
                MAX11611 max11611;
                uint16_t max11611Data[PRESSURE_SENSOR_COUNT];
                printf("\n.-.--..---.-.-.--.--.--.---.--.-\n");
                printf("TEST NO. : %c\n", testNoID);
                printf("Force sensors (ADC) validation\n");
                while (loopTest != 27)
                {
                    max11611.GetData(PRESSURE_SENSOR_COUNT, max11611Data);
                    for (uint8_t i = 0; i < PRESSURE_SENSOR_COUNT; i++)
                    {
                        forceSensor.SetAnalogData(i, max11611Data[i]);
                    }
                    printf("\nFORCE SENSORS VALUES\n");
                    printf("Sensor Number \t Analog value\n");
                    for (uint8_t i = 0; i < PRESSURE_SENSOR_COUNT; i++)
                    {
                        printf("Sensor No: %i \t %i\n", i + 1, (forceSensor.GetAnalogData(i)));
                    }
                    printf("\nENTER to repeat test\n");
                    printf("ESC+ENTER to exit test\n");
                    printf(".-.--..---.-.-.--.--.--.---.--.-\n");
                    loopTest = getchar();
                }
                getchar();
                break;
            }
            case '2':
            {
                printf("\n.-.--..---.-.-.--.--.--.---.--.-\n");
                printf("TEST NO. : %c\n", testNoID);
                printf("Force sensors calibration validation\n");
                while (loopTest != 27)
                {
                    PressureMat *pressureMat = PressureMat::GetInstance();
                    pressureMat->Calibrate();
                    pressureMat->Update();

                    printf("\nINITIAL OFFSET VALUES\n");
                    printf("Sensor Number\tAnalog Offset\n");
                    pressure_mat_offset_t pressureMatOffset = pressureMat->GetOffsets();
                    for (uint8_t i = 0; i < PRESSURE_SENSOR_COUNT; i++)
                    {
                        printf("Sensor No: %i \t %u \n", i + 1, pressureMatOffset.analogOffset[i]);
                    }
                    printf("\nSensor mean from calibration : \t %u \n", pressureMatOffset.totalSensorMean);
                    printf("Detection Threshold : %f \n", pressureMatOffset.detectionThreshold);

                    printf("\nENTER to repeat test\n");
                    printf("ESC+ENTER to exit test\n");
                    printf(".-.--..---.-.-.--.--.--.---.--.-\n");
                    loopTest = getchar();
                }
                getchar();
                break;
            }
            case '3':
            {
                printf("\n.-.--..---.-.-.--.--.--.---.--.-\n");
                printf("TEST NO. : %c\n", testNoID);
                printf("Presence detection validation\n");
                while (loopTest != 27)
                {
                    PressureMat *pressureMat = PressureMat::GetInstance();
                    pressureMat->Update();
                    pressure_mat_offset_t pressureMatOffset = pressureMat->GetOffsets();
                    uint16_t sensedPresence = 0;

                    for (uint8_t i = 0; i < PRESSURE_SENSOR_COUNT; i++)
                    {
                        sensedPresence += pressureMatOffset.analogOffset[i];
                    }
                    if (PRESSURE_SENSOR_COUNT != 0)
                    {
                        sensedPresence /= PRESSURE_SENSOR_COUNT;
                    }
                    printf("\nSensed presence (mean(Analog Value)) = %i\n", sensedPresence);
                    printf("Detection Threshold set to : %f \n", pressureMatOffset.detectionThreshold);
                    printf("Presence detection result : ");
                    if (forceSensor.IsUserDetected())
                    {
                        printf("User detected \n");
                    }
                    else
                    {
                        printf("No user detected \n");
                    }
                    printf("\nENTER to repeat test\n");
                    printf("ESC+ENTER to exit test\n");
                    printf(".-.--..---.-.-.--.--.--.---.--.-\n");
                    loopTest = getchar();
                }
                getchar();
                break;
            }
            case '4':
            {
                printf("\n.-.--..---.-.-.--.--.--.---.--.-\n");
                printf("TEST NO. : %c\n", testNoID);
                printf("Force plates validation\n");
                PressureMat *pressureMat = PressureMat::GetInstance();
                pressureMat->Calibrate();
                pressure_mat_data_t pressureMatData = pressureMat->GetPressureMatData();

                while (loopTest != 27)
                {
                    pressureMat->Update();
                    pressureMat->UpdateForcePlateData();
                    pressureMatData = pressureMat->GetPressureMatData();

                    printf("\nFORCE PLATES CENTER OF PRESSURES\n");
                    printf("Relative position of the center of pressure for each quadrants (inches) \n");
                    printf("COP Axis \t forcePlate1 \t forcePlate2 \t forcePlate3 \t forcePlate4 \n");
                    printf("COP (X): \t %f \t %f \t %f \t %f \n",
                           pressureMatData.quadrantPressure[0].x,
                           pressureMatData.quadrantPressure[1].x,
                           pressureMatData.quadrantPressure[2].x,
                           pressureMatData.quadrantPressure[3].x);
                    printf("COP (Y): \t %f \t\%f \t %f \t %f \n",
                           pressureMatData.quadrantPressure[0].y,
                           pressureMatData.quadrantPressure[1].y,
                           pressureMatData.quadrantPressure[2].y,
                           pressureMatData.quadrantPressure[3].y);
                    printf("\nENTER to repeat test\n");
                    printf("ESC+ENTER to exit test\n");
                    printf(".-.--..---.-.-.--.--.--.---.--.-\n");
                    loopTest = getchar();
                }
                getchar();
                break;
            }
            case '5':
            {
                printf("\n.-.--..---.-.-.--.--.--.---.--.-\n");
                printf("TEST NO. : %c\n", testNoID);
                printf("Center of pressure validation\n");
                PressureMat *pressureMat = PressureMat::GetInstance();
                pressureMat->Calibrate();
                pressure_mat_data_t pressureMatData = pressureMat->GetPressureMatData();

                while (loopTest != 27)
                {
                    pressureMat->Update();
                    pressureMat->UpdateForcePlateData();
                    pressureMatData = pressureMat->GetPressureMatData();

                    printf("\nGLOBAL PLATES CENTER OF PRESSURE\n");
                    printf("Relative position of the global center of pressure (inches) \n");
                    printf("COP (X): \t %f\n", pressureMatData.centerOfPressure.x);
                    printf("COP (Y): \t %f\n", pressureMatData.centerOfPressure.y);
                    printf("\nENTER to repeat test\n");
                    printf("ESC+ENTER to exit test\n");
                    printf(".-.--..---.-.-.--.--.--.---.--.-\n");
                    loopTest = getchar();
                }
                getchar();
                break;
            }
            case 'q':
            {
                subTestExit = 1;
                break;
            }
            default:
            {
                printf("\nInvalid TestID = %i\n", testNoID);
            }
            break;
            }
        }
        break;
    }
    case '5':
    {
        MotionSensor *motionSensor = MotionSensor::GetInstance();
        subTestExit = 0;

        while (subTestExit != 1)
        {
            printf("\n FLOW SENSOR MODULE\n");

            printf("\n.-.--..--- FLOW SENSOR MODULE MENU .--.---.--.-\n");
            printf("\nTestID\tDescription");
            printf("\n--------------------------------------");
            printf("\n 1 \t Range Sensor Validation");
            printf("\n 2 \t Flow Sensor Validation");
            printf("\n 3 \t Is Moving Validation");
            printf("\n q \t *Back to Main Test Menu");
            printf("\n--------------------------------------");
            printf("\nEnter a Test No. and press the return key to run test\n");
            testNoID = getchar();
            getchar(); // To consume '\n'
            loopTest = 0;

            switch (testNoID)
            {
            case '1':
            {
                printf("\n.-.--..---.-.-.--.--.--.---.--.-\n");
                printf("TEST NO. : %c\n", testNoID);
                printf("Range Sensor Validation\n");
                while (loopTest != 27)
                {
                    printf("\nAim flow sensor at floor and press ENTER");
                    getchar();

                    printf("Distance from floor (mm): %u \n", motionSensor->GetRangeSensorValue());

                    printf("\nENTER to repeat test\n");
                    printf("ESC+ENTER to exit test\n");
                    printf(".-.--..---.-.-.--.--.--.---.--.-\n");
                    loopTest = getchar();
                }
                getchar();
                break;
            }
            case '2':
            {
                printf("\n.-.--..---.-.-.--.--.--.---.--.-\n");
                printf("TEST NO. : %c\n", testNoID);
                printf("Flow Sensor Validation\n");
                while (loopTest != 27)
                {
                    for (uint8_t i = 0; i < 5; i++)
                    {
                        Coord_t deltaPixel = motionSensor->GetFlowSensorValues();
                        printf("Reading %i : Delta X: %f \tDelta Y: %f\n", i + 1, deltaPixel.x, deltaPixel.y);
                        sleep_for_milliseconds(1000);
                    }

                    printf("\nENTER to repeat test\n");
                    printf("ESC+ENTER to exit test\n");
                    printf(".-.--..---.-.-.--.--.--.---.--.-\n");
                    loopTest = getchar();
                }
                getchar();
                break;
            }
            case '3':
            {
                const uint8_t testTime = 10;
                printf("\n.-.--..---.-.-.--.--.--.---.--.-\n");
                printf("TEST NO. : %c\n", testNoID);
                printf("Slowly move the flow sensor for %u seconds.\n", testTime);
                while (loopTest != 27)
                {
                    bool isMoving = false;
                    for (uint8_t i = 0; i < testTime; i++)
                    {
                        motionSensor->GetDeltaXY();
                        printf("%i", testTime - i);
                        if (motionSensor->IsMoving())
                        {
                            isMoving = true;
                        }
                        sleep_for_milliseconds(1000);
                    }
                    printf("Movement detected: %s", isMoving ? "TRUE" : "FALSE");

                    printf("\nENTER to repeat test\n");
                    printf("ESC+ENTER to exit test\n");
                    printf(".-.--..---.-.-.--.--.--.---.--.-\n");
                    loopTest = getchar();
                }
                getchar();
                break;
            }
            case 'q':
            {
                subTestExit = 1;
                break;
            }
            default:
            {
                printf("\nInvalid TestID = %i\n", testNoID);
            }
            break;
            }
        }
    }
    case 'q':
    {
        return true;
    }
    default:
    {
        printf("\nInvalid Module ID = %i\n", moduleID);
    }
    }
    return false;
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
