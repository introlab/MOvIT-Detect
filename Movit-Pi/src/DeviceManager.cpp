#include "DeviceManager.h"
#include "NetworkManager.h"
#include "I2Cdev.h"
#include "Utils.h"

#include <unistd.h>
#include <thread>

DeviceManager::DeviceManager() : _alarm(700, 0.1)
{
    _datetimeRTC = DateTimeRTC::GetInstance();
    _COPCoord.x = 0;
    _COPCoord.y = 0;
}

void DeviceManager::InitializeDevices()
{
    I2Cdev::Initialize();
    _datetimeRTC->SetCurrentDateTimeIfConnectedThread().detach();

    _alarm.Initialize();
    _imuValid = _imu.Initialize();
    _motionSensor.Initialize();
    _forcePlateValid = InitializeForcePlate();

    printf("Setup Done\n");
}

void DeviceManager::TurnOff()
{
    GetAlarm()->TurnOffAlarm();
}

void DeviceManager::CalibratePressureMat()
{
    UpdateForcePlateData();
    _sensorMatrix.CalibrateForceSensor(_max11611Data, _max11611);
}

void DeviceManager::CalibrateIMU()
{
    _imu.Calibrate();
}

bool DeviceManager::InitializeForcePlate()
{
    printf("MAX11611 (ADC) initializing ... ");
    if (_max11611.Initialize())
    {
        for (uint8_t i = 0; i < _sensorMatrix._sensorCount; i++)
        {
            _sensorMatrix.SetAnalogData(0, i);
        }
        _sensorMatrix.GetForceSensorData();

        // TODO tester avec le tapis pour savoir si ce call devrait rester ici
        CalibratePressureMat();

        printf("success\n");
        return true;
    }
    else
    {
        printf("FAIL\n");
        return false;
    }
}

void DeviceManager::Update()
{
    _timeSinceEpoch = _datetimeRTC->GetTimeSinceEpoch();
    _isMoving = _motionSensor.GetIsMoving();

    if (_imuValid)
    {
        // Data: Angle (centrales intertielles mobile/fixe)
        _backSeatAngle = _backSeatAngleTracker.GetBackSeatAngle();
    }

    if (_forcePlateValid)
    {
        // Data: Capteur de force
        UpdateForcePlateData();

        _isSomeoneThere = _sensorMatrix.IsUserDetected();
        if (_isSomeoneThere)
        {
            _globalForcePlate.DetectCenterOfPressure(_globalForcePlate, _sensorMatrix);
            _COPCoord.x = _globalForcePlate.GetCOPx();
            _COPCoord.y = _globalForcePlate.GetCOPy();
        }
        else
        {
            _COPCoord.x = 0;
            _COPCoord.y = 0;
        }
    }
}

void DeviceManager::UpdateForcePlateData()
{
    _max11611.GetData(_sensorMatrix._sensorCount, _max11611Data);
    for (uint8_t i = 0; i < _sensorMatrix._sensorCount; i++)
    {
        _sensorMatrix.SetAnalogData(_max11611Data[i], i);
    }
    _sensorMatrix.GetForceSensorData();
}

bool DeviceManager::TestDevices()
{
    printf("\n.-.--..--- TEST MENU .--.---.--.-\n");
    printf("Select one of the following options.\n");

    printf("\n\nFunction testing :");
    printf("\n\tTestID\tDescription");
    printf("\n\t a \t Activate GREEN LED on notification module");
    printf("\n\t b \t Activate RED LED on notification module");
    printf("\n\t c \t Activate DC Motor on notification module");
    printf("\n\t d \t Activate blink leds alarm.");
    printf("\n\t e \t Activate red alarm.");
    printf("\n\t f \t Activate force sensors calibration");
    printf("\n\t g \t Check force sensors centers of pressure");
    printf("\n\t h \t De-activate all R&G LED + DC Motor");
    printf("\n\t i \t Activate IMU calibration");
    printf("\n\t j \t Detect relative pressure in quadrants");
    printf("\n\t k \t Print date and time");
    printf("\n\t q \t Close program");

    printf("\n\nType in TestID then press the return key\n");

    char inSerialChar = getchar();
    getchar(); // To consume '\n'

    //---------------------------------------------------------------------------------------
    // TEST DU MODULE NOTIFICATION : OK
    // DEL Red/Green, MoteurDC, Bouton poussoir
    //---------------------------------------------------------------------------------------

    if (inSerialChar == 'a')
    {
        printf("Allumer la DEL verte.\n");
        _alarm.TurnOnGreenLed();
    }
    else if (inSerialChar == 'b')
    {
        printf("Allumer la DEL rouge.\n");
        _alarm.TurnOnRedLed();
    }
    else if (inSerialChar == 'c')
    {
        printf("Activer le moteur DC.\n");
        uint8_t count = 0;
        while (count++ < 70)
        {
            _alarm.TurnOnDCMotor();
            sleep_for_microseconds(0.1 * 1000 * 1000);
        }
        _alarm.TurnOffDCMotor();
    }
    else if (inSerialChar == 'd')
    {
        printf("Activer une alarme <rouge/verte>.\n");
        _alarm.TurnOnBlinkLedsAlarm();
    }
    else if (inSerialChar == 'e')
    {
        printf("Activer une alarme <rouge/moteur>.\n");
        _alarm.TurnOnRedAlarm();
    }
    else if (inSerialChar == 'f')
    {
        printf("\nDEBUG CALIBRATION FORCE SENSORS START");
        printf("\nFunction(s) under test:");
        printf("\n CalibrateForceSensor()");
        printf("\n IsUserDetected()");

        //Calibration
        UpdateForcePlateData();
        _sensorMatrix.CalibrateForceSensor(_max11611Data, _max11611);
        //Last sensed presence analog reading to compare with calibration
        uint16_t sensedPresence = 0;
        for (uint8_t i = 0; i < _sensorMatrix._sensorCount; i++)
        {
            sensedPresence += _sensorMatrix.GetAnalogData(i);
        }
        if (_sensorMatrix._sensorCount != 0)
        {
            sensedPresence /= _sensorMatrix._sensorCount;
        }

        printf("\n.-.--..---DERNIERE MESURE DES CAPTEURS EN TEMPS REEL.--.---.--.-\n");
        printf("Sensor Number \t Analog value \t Voltage (mV) \t Force (N) \n");
        for (uint8_t i = 0; i < _sensorMatrix._sensorCount; i++)
        {
            printf("Sensor No: %i \t %i \t\t %u \t\t  %f \n", i + 1, _sensorMatrix.GetAnalogData(i), _sensorMatrix.GetVoltageData(i), _sensorMatrix.GetForceData(i));
        }
        printf(".-.--..---.-.-.--.--.--.---.--.-\n");

        printf("\n.-.--..---.-.OFFSET INITIAUX.--.---.--.-\n");
        printf("Sensor Number\t");
        printf("Analog value\n");
        for (uint8_t i = 0; i < _sensorMatrix._sensorCount; i++)
        {
            printf("Sensor No: %i \t %u \n", i + 1, _sensorMatrix.GetAnalogOffset(i));
        }
        printf(".-.--..---.-.-.--.--.--.---.--.-\n");

        printf("\n.-.--..---.-.OFFSET INITIAUX.--.---.--.-\n");
        printf("Total sensor mean : \t %i \n", _sensorMatrix.GetTotalSensorMean());
        printf(".-.--..---.-.-.--.--.--.---.--.-\n");

        printf("\n.-.--..---DETECTION DUNE PERSONNE SUR LA CHAISE--.---.--.-\n");
        printf("Detected Presence : %u \n", sensedPresence);
        printf("Detection Threshold : %f \n", _sensorMatrix.GetDetectionThreshold());
        printf("Presence verification result : ");

        if (_sensorMatrix.IsUserDetected())
        {
            printf("User detected \n");
        }
        else
        {
            printf("No user detected \n");
        }
        printf(".-.--..---.-.-.--.--.--.---.--.-\n\n");
    }
    else if (inSerialChar == 'g')
    {
        uint16_t sensedPresence = 0;
        for (uint8_t i = 0; i < _sensorMatrix._sensorCount; i++)
        {
            sensedPresence += _sensorMatrix.GetAnalogData(i);
        }
        if (_sensorMatrix._sensorCount != 0)
        {
            sensedPresence /= _sensorMatrix._sensorCount;
        }

        printf("\nDEBUG CENTER OF PRESSURE FORCE SENSORS START");
        printf("\nFunction(s) under test:");
        printf("\n DetectCenterOfPressure()");
        printf("\n\t CreateForcePlate()");
        printf("\n\t AnalyseForcePlate()");

        UpdateForcePlateData();
        _globalForcePlate.DetectCenterOfPressure(_globalForcePlate, _sensorMatrix);

        printf("\n.-.--..---MESURE DES CAPTEURS DE FORCE--.---.--.-\n");
        printf("Sensor Number \t Analog value \t Voltage (mV) \t Force (N) \n");
        for (uint8_t i = 0; i < _sensorMatrix._sensorCount; i++)
        {
            printf("Sensor No: %i \t %i \t\t %u \t\t %f \n", i + 1, _sensorMatrix.GetAnalogData(i), _sensorMatrix.GetVoltageData(i), _sensorMatrix.GetForceData(i));
        }
        printf(".-.--..---.-.-.--.--.--.---.--.-\n");

        printf("\n.-.--..---MESURE DES CENTRES DE PRESSION.--.---.--.-\n");
        printf("Position relative au centre des quadrants sur le siege (cm) \n");
        printf("COP Axis \t forcePlate1 \t forcePlate2 \t forcePlate3 \t forcePlate4 \n");
        printf("COP (X): \t %f \t %f \t %f \t %f \n", _globalForcePlate.GetFp1COPx(), _globalForcePlate.GetFp2COPx(), _globalForcePlate.GetFp3COPx(), _globalForcePlate.GetFp4COPx());
        printf("COP (Y): \t %f \t\%f \t %f \t %f \n", _globalForcePlate.GetFp1COPy(), _globalForcePlate.GetFp2COPy(), _globalForcePlate.GetFp3COPy(), _globalForcePlate.GetFp4COPy());

        printf("\nPosition globale relative au centre du siege (cm) \n");
        printf("COP Axis \t globalForcePlate \n");
        printf("COP (X): \t %f \n", _globalForcePlate.GetCOPx());
        printf("COP (Y): \t %f \n", _globalForcePlate.GetCOPy());
        printf(".-.--..---.-.-.--.--.--.---.--.-");
        printf("\n");

        printf("\n.-.--..---DETECTION DUNE PERSONNE SUR LA CHAISE--.---.--.-\n");
        printf("Detected Presence : %u \n", sensedPresence);
        printf("Detection Threshold : %f \n", _sensorMatrix.GetDetectionThreshold());
        printf("Presence verification result : ");

        if (_sensorMatrix.IsUserDetected())
        {
            printf("User detected \n");
        }
        else
        {
            printf("No user detected \n");
        }
        printf(".-.--..---.-.-.--.--.--.---.--.-\n\n");
    }
    else if (inSerialChar == 'h')
    {
        printf("Eteindre les DELs et arrêter le moteur DC.\n");
        // _alarm.TurnOffAlarm();
        _backSeatAngleTracker.GetBackSeatAngle();
    }
    else if (inSerialChar == 'i')
    {
        //_imu.SetCalibrationArray(fixedImu, 0);
        delay(50);
        //_imu.SetCalibrationArray(imuMobile, 0);
        printf("Calibration des capteurs effectuée.\n");
        inSerialChar = 'x';
    }
    else if (inSerialChar == 'j')
    {
        printf("\nDEBUG RELATIVE PRESSURE IN QUADRANTS START");
        printf("\nFunction(s) under test:");
        printf("\n DetectRelativePressure()\n");

        UpdateForcePlateData();

        printf("\n.-.--..---MESURE DES PRESSIONS RELATIVES DES QUADRANTS--.---.--.-\n");
        int *relativePressureLevel = _sensorMatrix.DetectRelativePressure();
        for (int i = 0; i < 4; i++)
        {
            printf("\nQuadrant %d : ", (i + 1));
            //printf(" %d \n", *(relativePressureLevel + i));
            if (*(relativePressureLevel + i) == 1)
            {
                printf("Really low");
            }
            else if (*(relativePressureLevel + i) == 2)
            {
                printf("Low");
            }
            else if (*(relativePressureLevel + i) == 3)
            {
                printf("Normal");
            }
            else if (*(relativePressureLevel + i) == 4)
            {
                printf("High");
            }
            else if (*(relativePressureLevel + i) == 5)
            {
                printf("Really high");
            }
            else if (*(relativePressureLevel + i) == 0)
            {
                printf("No pressure data to read");
            }
            else
            {
                //Reading error
            }
        }
        printf("\n");
    }
    else if (inSerialChar == 'k')
    {
        int timeSinceEpoch = _datetimeRTC->GetTimeSinceEpoch();
        printf("Time since epoch: %d\n", timeSinceEpoch);
    }
    else if (inSerialChar == 'q')
    {
        return true;
    }

    return false;
}
