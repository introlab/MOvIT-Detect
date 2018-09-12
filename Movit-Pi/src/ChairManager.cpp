#include "ChairManager.h"
#include <chrono>

#define REQUIRED_SITTING_TIME 5
#define DELTA_ANGLE_THRESHOLD 5

#define KEEP_ALIVE_PERIOD 300000
#define VIBRATION_EMISSION_PERIOD 1000
#define MINIMUM_BACK_REST_ANGLE 2

#define IS_MOVING_DEBOUNCE_CONSTANT 10

using std::chrono::duration_cast;
using std::chrono::milliseconds;
using std::chrono::seconds;

ChairManager::ChairManager(MosquittoBroker *mosquittoBroker, DeviceManager *devicemgr)
    : _mosquittoBroker(mosquittoBroker), _devicemgr(devicemgr)
{
    _copCoord.x = 0;
    _copCoord.y = 0;
}

ChairManager::~ChairManager()
{
}

void ChairManager::UpdateDevices()
{
    _currentDatetime = std::to_string(_devicemgr->GetTimeSinceEpoch());

    if (_mosquittoBroker->CalibPressureMatRequired())
    {
        printf("Debut de la calibration du tapis de pression\n");
        _devicemgr->CalibratePressureMat();
        _mosquittoBroker->SendIsPressureMatCalib(true, _currentDatetime);
        printf("FIN de la calibration du tapis de pression\n");
    }

    if (_mosquittoBroker->CalibIMURequired())
    {
        printf("Debut de la calibration des IMU\n");
        _devicemgr->CalibrateIMU();
        _mosquittoBroker->SendIsIMUCalib(true, _currentDatetime);
        printf("FIN de la calibration des IMU\n");
    }

    _prevIsSomeoneThere = _isSomeoneThere;

    _devicemgr->Update();
    _isSomeoneThere = _devicemgr->IsSomeoneThere();
    _copCoord = _devicemgr->GetCenterOfPressure();
    _currentChairAngle = _devicemgr->GetBackSeatAngle();
    _isMoving = _devicemgr->GetIsMoving();

#ifdef DEBUG_PRINT
    //printf("getDateTime = %s\n", _currentDatetime.c_str());
    printf("\n");
    printf("isSomeoneThere = %i\n", _devicemgr->IsSomeoneThere());
    printf("getCenterOfPressure x = %f, y = %f\n", _copCoord.x, _copCoord.y);
    printf("_currentChairAngle = %i\n", _currentChairAngle);
    printf("_isMoving = %i\n", _isMoving);
    //printf("_prevChairAngle = %i\n\n", _prevChairAngle);
    //printf("Current Speed = %f\n\n", _currentSpeed);
#endif

    if (_isSomeoneThere)
    {
        if (_centerOfPressureTimer.Elapsed() >= CENTER_OF_PRESSURE_EMISSION_PERIOD.count())
        {
            _centerOfPressureTimer.Reset();
            _mosquittoBroker->SendCenterOfPressure(_copCoord.x, _copCoord.y, _currentDatetime);
        }

        if ((_chairAngleTimer.Elapsed() >= CHAIR_ANGLE_EMISSION_PERIOD.count()) && (_currentChairAngle != _prevChairAngle))
        {
            _prevChairAngle = _currentChairAngle;
            _chairAngleTimer.Reset();
            _mosquittoBroker->SendBackRestAngle(_currentChairAngle, _currentDatetime);
        }
    }

    if (_keepAliveTimer.Elapsed() >= KEEP_ALIVE_PERIOD)
    {
        _keepAliveTimer.Reset();
        _mosquittoBroker->SendKeepAlive(_currentDatetime);
    }

    if (_vibrationTimer.Elapsed() >= VIBRATION_EMISSION_PERIOD)
    {
        double acceleration = _devicemgr->GetXAcceleration();
        printf("getXAcceleration (vibration) = %f\n", acceleration);
        _vibrationTimer.Reset();
        _mosquittoBroker->SendVibration(acceleration, _currentDatetime);
    }

    if (_prevIsSomeoneThere != _isSomeoneThere)
    {
        _mosquittoBroker->SendIsSomeoneThere(_isSomeoneThere, _currentDatetime);
    }
}

void ChairManager::ReadFromServer()
{
    if (_mosquittoBroker->IsSetAlarmOnNew())
    {
        _overrideNotificationPattern = true;
        _setAlarmOn = _mosquittoBroker->GetSetAlarmOn();
        printf("Something new for setAlarmOn = %i\n", _setAlarmOn);
    }
    if (_mosquittoBroker->IsRequiredBackRestAngleNew())
    {
        _requiredBackRestAngle = _mosquittoBroker->GetRequiredBackRestAngle();
        _secondsCounter = 0;
        // TODO valider que c'est le bon _state
        _state = 1;
        printf("Something new for _requiredBackRestAngle = %i\n", _requiredBackRestAngle);
    }
    if (_mosquittoBroker->IsRequiredPeriodNew())
    {
        _requiredPeriod = _mosquittoBroker->GetRequiredPeriod();
        _secondsCounter = 0;
        // TODO valider que c'est le bon _state
        _state = 1;
        printf("Something new for _requiredPeriod = %i\n", _requiredPeriod);
    }
    if (_mosquittoBroker->IsRequiredDurationNew())
    {
        _requiredDuration = _mosquittoBroker->GetRequiredDuration();
        _secondsCounter = 0;
        // TODO valider que c'est le bon _state
        _state = 1;
        printf("Something new for _requiredDuration = %i\n", _requiredDuration);
    }
}

void ChairManager::CheckNotification()
{
    if (_overrideNotificationPattern)
    {
        OverrideNotificationPattern();
        return;
    }

    if (!_isSomeoneThere || _requiredDuration == 0 || _requiredPeriod == 0 || _requiredBackRestAngle == 0)
    {
        _state = 1;
        _secondsCounter = 0;
        _devicemgr->GetAlarm()->TurnOffAlarm();
        return;
    }

    switch (_state)
    {
    case 1:
        CheckIfUserHasBeenSittingForRequiredTime();
        break;
    case 2:
        CheckIfBackRestIsRequired();
        break;
    case 3:
        CheckIfRequiredBackSeatAngleIsReached();
        break;
    case 4:
        CheckIfRequiredBackSeatAngleIsMaintained();
        break;
    case 5:
        CheckIfBackSeatIsBackToInitialPosition();
        break;
    default:
        break;
    }
}

void ChairManager::CheckIfUserHasBeenSittingForRequiredTime()
{
    printf("State 1\t_secondsCounter: %i\n", _secondsCounter);

    if (++_secondsCounter >= REQUIRED_SITTING_TIME)
    {
        _state = 2;
        _secondsCounter = 0;
    }
}

void ChairManager::CheckIfBackRestIsRequired()
{
    printf("State 2\t_secondsCounter: %i\n", _secondsCounter);

    if (++_secondsCounter >= _requiredPeriod)
    {
        if (!_isMoving)
        {
            _devicemgr->GetAlarm()->StopBlinkGreenAlarm();
            _devicemgr->GetAlarm()->TurnOnRedAlarmThread().detach();
            _state = 3;
            _secondsCounter = 0;
        }
        else
        {
            _devicemgr->GetAlarm()->TurnOnBlinkGreenAlarmThread().detach();
        }
    }
}

void ChairManager::CheckIfRequiredBackSeatAngleIsReached()
{
    printf("State 3\t abs(requiredBackRestAngle - _currentChairAngle): %i\n", abs(int(_requiredBackRestAngle) - int(_currentChairAngle)));

    if (_currentChairAngle > (_requiredBackRestAngle - DELTA_ANGLE_THRESHOLD))
    {
        _devicemgr->GetAlarm()->TurnOnGreenAlarm();
        _state = 4;
    }
}

void ChairManager::CheckIfRequiredBackSeatAngleIsMaintained()
{
    printf("State 4\n");

    if (_currentChairAngle > (_requiredBackRestAngle - DELTA_ANGLE_THRESHOLD))
    {
        printf("State 4\t_secondsCounter: %i\n", _secondsCounter);

        if (++_secondsCounter >= _requiredDuration)
        {
            _devicemgr->GetAlarm()->TurnOnBlinkLedsAlarmThread().detach();
            _state = 5;
        }
    }
    else
    {
        printf("State 4 il faut remonter\n");
        _devicemgr->GetAlarm()->TurnOnRedAlarmThread().detach();
        _state = 3;
        _secondsCounter = 0;
    }
}

void ChairManager::CheckIfBackSeatIsBackToInitialPosition()
{
    printf("State 5\t_secondsCounter: %i\n", ++_secondsCounter);

    if (_currentChairAngle < (_requiredBackRestAngle - DELTA_ANGLE_THRESHOLD))
    {
        _state = 2;
        _secondsCounter = 0;
    }
}

void ChairManager::OverrideNotificationPattern()
{
    if (_setAlarmOn)
    {
        _devicemgr->GetAlarm()->TurnOnRedLed();
        _devicemgr->GetAlarm()->TurnOnGreenLed();
        _devicemgr->GetAlarm()->TurnOnDCMotor();
    }
    else
    {
        _devicemgr->GetAlarm()->TurnOffAlarm();
    }
    _overrideNotificationPattern = false;
}
