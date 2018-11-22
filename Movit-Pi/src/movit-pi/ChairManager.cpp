#include "ChairManager.h"
#include <chrono>
#include "NetworkManager.h"
#include "SysTime.h"

#define REQUIRED_SITTING_TIME 5
#define DELTA_ANGLE_THRESHOLD 5

#define CHECK_SENSORS_STATE_PERIOD 20
#define VIBRATION_EMISSION_FREQUENCY 60 // Hz
#define VIBRATION_EMISSION_THRESOLD 2   // m/s^2

using std::chrono::duration_cast;
using std::chrono::milliseconds;
using std::chrono::seconds;

enum State
{
    INITIAL = 1,
    WAIT,
    SNOOZE,
    CLIMB,
    STAY,
    DESCEND,
    INVALID
};

enum TiltInfo
{
    SUCCESS = 0,
    TOO_SHORT,
    FAIL,
    SNOOZED,
    TOO_LOW,
    COUNT
};

ChairManager::ChairManager(MosquittoBroker *mosquittoBroker, DeviceManager *deviceManager) : _mosquittoBroker(mosquittoBroker),
                                                                                             _deviceManager(deviceManager),
                                                                                             _secondsCounter(RUNNING_FREQUENCY)
{
    _alarm = _deviceManager->GetAlarm();
}

ChairManager::~ChairManager()
{
}

void ChairManager::UpdateDevices()
{
    _deviceManager->Update();

    _currentDatetime = std::to_string(_deviceManager->GetTimeSinceEpoch());

    if (_mosquittoBroker->CalibPressureMatRequired())
    {
        _deviceManager->CalibratePressureMat();
        _isPressureMatCalibrationChanged = true;
    }
    if (_deviceManager->IsPressureMatCalibrated() && _isPressureMatCalibrationChanged)
    {
        _mosquittoBroker->SendIsPressureMatCalib(true, _currentDatetime);
        _isPressureMatCalibrationChanged = false;
    }

    if (_mosquittoBroker->CalibIMURequired())
    {
        _deviceManager->CalibrateIMU();
        _isIMUCalibrationChanged = true;
    }
    if (_deviceManager->IsImuCalibrated() && _isIMUCalibrationChanged)
    {
        _mosquittoBroker->SendIsIMUCalib(true, _currentDatetime);
        _isIMUCalibrationChanged = false;
    }

    _prevIsSomeoneThere = _isSomeoneThere;
    _isSomeoneThere = _deviceManager->IsSomeoneThere();
    _prevChairAngle = _currentChairAngle;
    _currentChairAngle = _deviceManager->GetBackSeatAngle();
    bool prevIsMoving = _isMoving;
    _isMoving = _deviceManager->IsMoving();
    _isChairInclined = _deviceManager->IsChairInclined();
    _pressureMatData = _deviceManager->GetPressureMatData();

#ifdef DEBUG_PRINT
    printf("\n");
    printf("_currentDatetime = %s\n", _currentDatetime.c_str());
    printf("isSomeoneThere = %i\n", _isSomeoneThere);
    printf("_currentChairAngle = %i\n", _currentChairAngle);
    printf("_isMoving = %i\n", _isMoving);
    printf("_isChairInclined = %i\n", _isChairInclined);
    printf("_snoozeTime = %f\n", _snoozeTime);
    printf("Global Center Of Pressure = X: %f Y: %f\n", _pressureMatData.centerOfPressure.x, _pressureMatData.centerOfPressure.y);
#endif

    if (_isSomeoneThere)
    {
        if (_centerOfPressureTimer.Elapsed() >= CENTER_OF_PRESSURE_EMISSION_PERIOD.count())
        {
            _centerOfPressureTimer.Reset();
            _mosquittoBroker->SendPressureMatData(_pressureMatData, _currentDatetime);
        }

        if ((_chairAngleTimer.Elapsed() >= CHAIR_ANGLE_EMISSION_PERIOD.count()) && (_currentChairAngle != _prevChairAngle))
        {
            _chairAngleTimer.Reset();
            _mosquittoBroker->SendBackRestAngle(_currentChairAngle, _currentDatetime);
        }
    }

    if (_wifiChangedTimer.Elapsed() >= WIFI_VALIDATION_PERIOD.count() && _isWifiChanged)
    {
        _isWifiChanged = false;
        _mosquittoBroker->SendIsWifiConnected(NetworkManager::IsConnected(), _currentDatetime);
    }

    if (_heartbeatTimer.Elapsed() >= HEARTBEAT_PERIOD.count())
    {
        _heartbeatTimer.Reset();
        _mosquittoBroker->SendHeartbeat(_currentDatetime);
    }

    if (_prevIsSomeoneThere != _isSomeoneThere)
    {
        _mosquittoBroker->SendIsSomeoneThere(_isSomeoneThere, _currentDatetime);
    }

    if (_isMoving != prevIsMoving)
    {
        _mosquittoBroker->SendIsMoving(_isMoving, _currentDatetime);
    }

    _mosquittoBroker->SendSensorsState(_deviceManager->GetSensorState(), _currentDatetime);
}

void ChairManager::ReadVibrations()
{
    while (_isVibrationsActivated)
    {
        double acceleration = _deviceManager->GetXAcceleration();
        if (acceleration > VIBRATION_EMISSION_THRESOLD || acceleration < -VIBRATION_EMISSION_THRESOLD)
        {
            _mosquittoBroker->SendVibration(acceleration, _currentDatetime);
        }
        sleep_for_milliseconds(SECONDS_TO_MILLISECONDS / VIBRATION_EMISSION_FREQUENCY);
    }
}

void ChairManager::SetVibrationsActivated(bool isVibrationsActivated)
{
    _isVibrationsActivated = isVibrationsActivated;
}

std::thread ChairManager::ReadVibrationsThread()
{
    return std::thread([=] {
        ReadVibrations();
    });
}

void ChairManager::ReadFromServer()
{
    if (_mosquittoBroker->IsSetAlarmOnNew())
    {
        _setAlarmOn = _mosquittoBroker->GetSetAlarmOn();
        printf("Something new for setAlarmOn = %i\n", _setAlarmOn);
    }
    if (_mosquittoBroker->IsTiltSettingsChanged())
    {
        _tiltSettings = _mosquittoBroker->GetTiltSettings();
        _deviceManager->UpdateTiltSettings(_tiltSettings);
        _secondsCounter = 0;
        _state = State::INITIAL;
        printf("Something new for tilt settings\n");
    }
    if (_mosquittoBroker->IsWifiChanged())
    {
        NetworkManager::ChangeNetwork(_mosquittoBroker->GetWifiInformation());
        _isWifiChanged = true;
        _wifiChangedTimer.Reset();
    }

    if (_mosquittoBroker->IsNotificationsSettingsChanged())
    {
        notifications_settings_t notificationsSettings = _mosquittoBroker->GetNotificationsSettings();
        _deviceManager->UpdateNotificationsSettings(notificationsSettings);

        _snoozeTime = notificationsSettings.snoozeTime * 60.0f;
        _deviceManager->GetAlarm()->DeactivateVibration(!notificationsSettings.isVibrationEnabled);
        _deviceManager->GetAlarm()->DeactivateLedBlinking(!notificationsSettings.isLedBlinkingEnabled);
    }
}

void ChairManager::CheckNotification()
{
    if (_setAlarmOn)
    {
        OverrideNotificationPattern();
        return;
    }

    if (!_isSomeoneThere || _tiltSettings.requiredDuration == 0 || _tiltSettings.requiredPeriod == 0 || _tiltSettings.requiredBackRestAngle == 0 || _isMoving)
    {
        _state = State::INITIAL;
        _secondsCounter = 0;
        _alarm->TurnOffAlarm();
        return;
    }

    switch (_state)
    {
    case State::INITIAL:
        CheckIfUserHasBeenSittingForRequiredTime();
        break;
    case State::WAIT:
        CheckIfBackRestIsRequired();
        break;
    case State::SNOOZE:
        NotificationSnoozed();
        break;
    case State::CLIMB:
        CheckIfRequiredBackSeatAngleIsReached();
        break;
    case State::STAY:
        CheckIfRequiredBackSeatAngleIsMaintained();
        break;
    case State::DESCEND:
        CheckIfBackSeatIsBackToInitialPosition();
        break;
    default:
        break;
    }
}

void ChairManager::CheckIfUserHasBeenSittingForRequiredTime()
{
    printf("State INIT\t_secondsCounter: %f\n", _secondsCounter.Value());

    if (++_secondsCounter >= REQUIRED_SITTING_TIME)
    {
        _state = State::WAIT;
        _secondsCounter = 0;
    }
}
void ChairManager::NotificationSnoozed()
{
    _secondsCounter++;

    printf("State SNOOZED\t_secondsCounter: %f\n", _secondsCounter.Value());

    if (_secondsCounter >= _snoozeTime)
    {
        _secondsCounter = 0;
        _state = State::CLIMB;
        _failedTiltTimer.Reset();
    }
}

void ChairManager::CheckIfBackRestIsRequired()
{
    _secondsCounter++;

    printf("State WAIT\t_secondsCounter: %f\n", _secondsCounter.Value());

    if (_secondsCounter >= _tiltSettings.requiredPeriod && _currentChairAngle < MINIMUM_ANGLE)
    {
        if (!_isMoving && !_isChairInclined)
        {
            _alarm->StopBlinkGreenAlarm();
            _alarm->TurnOffRedLed();
            if (!_alarm->IsRedAlarmOn())
            {
                _alarm->TurnOnBlinkRedAlarmThread().detach();
            }
            _state = State::CLIMB;
            _secondsCounter = 0;

            _failedTiltTimer.Reset();
        }
        else if (!_alarm->IsBlinkGreenAlarmOn())
        {
            _alarm->TurnOnBlinkGreenAlarmThread().detach();
        }
    }
}

void ChairManager::CheckIfRequiredBackSeatAngleIsReached()
{
    printf("State CLIMB\t abs(requiredBackRestAngle - _currentChairAngle): %i\n", abs(int(_tiltSettings.requiredBackRestAngle) - int(_currentChairAngle)));

    if ((_failedTiltTimer.Elapsed() > FAILED_TILT_TIME.count()) && (_currentChairAngle <= MINIMUM_ANGLE))
    {
        printf("Failed bascule\n");
        _state = State::WAIT;
        _secondsCounter = 0;
        _alarm->StopBlinkRedAlarm();
        _mosquittoBroker->SendTiltInfo(TiltInfo::FAIL, _currentDatetime);
    }

    if (_prevChairAngle < MINIMUM_ANGLE && _currentChairAngle >= MINIMUM_ANGLE)
    {
        _failedTiltTimer.Reset();
    }

    if ((_failedTiltTimer.Elapsed() > (_tiltSettings.requiredDuration * SECONDS_TO_MILLISECONDS)) &&
        (_currentChairAngle > MINIMUM_ANGLE && _currentChairAngle <= _tiltSettings.requiredBackRestAngle))
    {
        printf("Tilt too low\n");
        _state = State::WAIT;
        _secondsCounter = 0;
        _alarm->StopBlinkRedAlarm();
        _alarm->TurnOnRedLed();
        _mosquittoBroker->SendTiltInfo(TiltInfo::TOO_LOW, _currentDatetime);
    }

    if (_alarm->ButtonPressed())
    {
        printf("Notification snoozed\n");
        _state = State::SNOOZE;
        _secondsCounter = 0;
        _alarm->TurnOffAlarm();
        _mosquittoBroker->SendTiltInfo(TiltInfo::SNOOZED, _currentDatetime);
    }

    if (_currentChairAngle > _tiltSettings.requiredBackRestAngle)
    {
        _alarm->StopBlinkRedAlarm();
        _alarm->TurnOnGreenAlarm();
        _state = State::STAY;
    }
}

void ChairManager::CheckIfRequiredBackSeatAngleIsMaintained()
{
    printf("State STAY\n");

    if (_currentChairAngle >= (_tiltSettings.requiredBackRestAngle - DELTA_ANGLE_THRESHOLD))
    {
        _secondsCounter++;

        printf("State STAY\t_secondsCounter: %f\n", _secondsCounter.Value());

        if (_secondsCounter >= _tiltSettings.requiredDuration)
        {
            if (!_alarm->IsBlinkGreenAlarmOn())
            {
                _alarm->TurnOnBlinkLedsAlarmThread().detach();
            }
            _state = State::DESCEND;
        }
    }
    else if (_currentChairAngle < MINIMUM_ANGLE)
    {
        printf("Tilt too short\n");
        _state = State::WAIT;
        _mosquittoBroker->SendTiltInfo(TiltInfo::TOO_SHORT, _currentDatetime);
    }
}

void ChairManager::CheckIfBackSeatIsBackToInitialPosition()
{
    _secondsCounter++;

    printf("State DESCEND\t_secondsCounter: %f\n", _secondsCounter.Value());

    if (_currentChairAngle < MINIMUM_ANGLE)
    {
        _alarm->StopBlinkLedsAlarm();
        _state = State::WAIT;
        _secondsCounter = 0;

        printf("Successsssss\n");

        _mosquittoBroker->SendTiltInfo(TiltInfo::SUCCESS, _currentDatetime);
    }
}

void ChairManager::OverrideNotificationPattern()
{
    if (_setAlarmOn)
    {
        _alarm->TurnOnRedLed();
        _alarm->TurnOnGreenLed();
        _alarm->TurnOnDCMotor();
    }
    else
    {
        _alarm->TurnOffAlarm();
    }
}
