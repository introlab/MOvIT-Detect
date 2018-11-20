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

void ChairManager::SendSensorsState()
{
    _currentDatetime = std::to_string(_deviceManager->GetTimeSinceEpoch());

    const bool isAlarmConnected = _deviceManager->IsAlarmConnected();
    const bool isFixedImuConnected = _deviceManager->IsFixedImuConnected();
    const bool isMobileImuConnected = _deviceManager->IsMobileImuConnected();
    const bool isMotionSensorConnected = _deviceManager->IsMotionSensorConnected();
    const bool isForcePlateConnected = _deviceManager->IsForcePlateConnected();

    _mosquittoBroker->SendSensorsState(isAlarmConnected, isFixedImuConnected, isMobileImuConnected, isMotionSensorConnected, isForcePlateConnected, _currentDatetime);
}

void ChairManager::UpdateSensor(int device, bool isConnected)
{
    if (_deviceManager->IsSensorStateChanged(device))
    {
        _mosquittoBroker->SendSensorState(device, isConnected, _currentDatetime);
    }
    _deviceManager->ReconnectSensor(device);
}

void ChairManager::UpdateDevices()
{
    _deviceManager->Update();

    _currentDatetime = std::to_string(_deviceManager->GetTimeSinceEpoch());

    // Needs refactoring
    if (++_updateDevicesCounter > CHECK_SENSORS_STATE_PERIOD)
    {
        UpdateSensor(DEVICES::alarmSensor, _deviceManager->IsAlarmConnected());
        UpdateSensor(DEVICES::fixedImu, _deviceManager->IsFixedImuConnected());
        UpdateSensor(DEVICES::mobileImu, _deviceManager->IsMobileImuConnected());
        // UpdateSensor(DEVICES::motionSensor, _deviceManager->IsMotionSensorConnected());
        _updateDevicesCounter = 0;
    }

    if (_mosquittoBroker->IsNotificationsSettingsChanged())
    {
        std::string notificationsSettingsStr =_mosquittoBroker->GetNotificationsSettings();
        _deviceManager->UpdateNotificationsSettings(notificationsSettingsStr);
    }

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
    _currentChairAngle = _deviceManager->GetBackSeatAngle();
    bool prevIsMoving = _isMoving;
    _isMoving = _deviceManager->IsMoving();
    _isChairInclined = _deviceManager->IsChairInclined();
    _pressureMatData = _deviceManager->GetPressureMatData();
    _prevChairAngle = _currentChairAngle;

#ifdef DEBUG_PRINT
    printf("\n");
    printf("_currentDatetime = %s\n", _currentDatetime.c_str());
    printf("isSomeoneThere = %i\n", _isSomeoneThere);
    printf("_currentChairAngle = %i\n", _currentChairAngle);
    printf("_isMoving = %i\n", _isMoving);
    printf("_isChairInclined = %i\n", _isChairInclined);
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
        _overrideNotificationPattern = true;
        _setAlarmOn = _mosquittoBroker->GetSetAlarmOn();
        printf("Something new for setAlarmOn = %i\n", _setAlarmOn);
    }
    if (_mosquittoBroker->IsRequiredBackRestAngleNew())
    {
        _requiredBackRestAngle = _mosquittoBroker->GetRequiredBackRestAngle();
        _secondsCounter = 0;
        _state = State::INITIAL;
        printf("Something new for _requiredBackRestAngle = %i\n", _requiredBackRestAngle);
    }
    if (_mosquittoBroker->IsRequiredPeriodNew())
    {
        _requiredPeriod = _mosquittoBroker->GetRequiredPeriod();
        _secondsCounter = 0;
        _state = State::INITIAL;
        printf("Something new for _requiredPeriod = %i\n", _requiredPeriod);
    }
    if (_mosquittoBroker->IsRequiredDurationNew())
    {
        _requiredDuration = _mosquittoBroker->GetRequiredDuration();
        _secondsCounter = 0;
        _state = State::INITIAL;
        printf("Something new for _requiredDuration = %i\n", _requiredDuration);
    }
    if (_mosquittoBroker->IsWifiChanged())
    {
        NetworkManager::ChangeNetwork(_mosquittoBroker->GetWifiInformation());
        _isWifiChanged = true;
        _wifiChangedTimer.Reset();
    }

    _snoozeTime = _deviceManager->GetSnoozeTime();
    _deviceManager->GetAlarm()->DeactivateVibration(_deviceManager->IsVibrationEnabled());
    _deviceManager->GetAlarm()->DeactivateLedBlinking(_deviceManager->IsLedBlinkingEnabled());
}

void ChairManager::CheckNotification()
{
    if (_overrideNotificationPattern)
    {
        OverrideNotificationPattern();
        return;
    }

    if (!_isSomeoneThere || _requiredDuration == 0 || _requiredPeriod == 0 || _requiredBackRestAngle == 0 || _isMoving)
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
        _state = State::WAIT;
    }
}

void ChairManager::CheckIfBackRestIsRequired()
{
    _secondsCounter++;

    printf("State WAIT\t_secondsCounter: %f\n", _secondsCounter.Value());

    if (_secondsCounter >= _requiredPeriod && _currentChairAngle < MINIMUM_ANGLE)
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
    printf("State CLIMB\t abs(requiredBackRestAngle - _currentChairAngle): %i\n", abs(int(_requiredBackRestAngle) - int(_currentChairAngle)));

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

    if ((_failedTiltTimer.Elapsed() > (_requiredDuration * SECONDS_TO_MILLISECONDS)) &&
        (_currentChairAngle > MINIMUM_ANGLE && _currentChairAngle <= _requiredBackRestAngle))
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

    if (_currentChairAngle > _requiredBackRestAngle)
    {
        _alarm->StopBlinkRedAlarm();
        _alarm->TurnOnGreenAlarm();
        _state = State::STAY;
    }
}

void ChairManager::CheckIfRequiredBackSeatAngleIsMaintained()
{
    printf("State STAY\n");

    if (_currentChairAngle >= (_requiredBackRestAngle - DELTA_ANGLE_THRESHOLD))
    {
        _secondsCounter++;

        printf("State STAY\t_secondsCounter: %f\n", _secondsCounter.Value());

        if (_secondsCounter >= _requiredDuration)
        {
            if (!_alarm->IsBlinkGreenAlarmOn())
            {
                _alarm->TurnOnBlinkLedsAlarmThread().detach();
            }
            _state = State::DESCEND;
        }
    }
    else if (_currentChairAngle < (_requiredBackRestAngle - DELTA_ANGLE_THRESHOLD))
    {
        printf("State STAY climb UP\n");
        if (!_alarm->IsRedAlarmOn())
        {
            _alarm->TurnOnBlinkRedAlarmThread().detach();
        }
        _state = State::CLIMB;
        _secondsCounter = 0;
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
    _overrideNotificationPattern = false;
}
