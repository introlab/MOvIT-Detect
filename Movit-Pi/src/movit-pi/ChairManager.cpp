#include "ChairManager.h"
#include <chrono>
#include "NetworkManager.h"

#define REQUIRED_SITTING_TIME 5
#define DELTA_ANGLE_THRESHOLD 5

#define VIBRATION_EMISSION_FREQUENCY 60 // Hz
#define VIBRATION_EMISSION_THRESOLD 2   // m/s^2
#define MINIMUM_BACK_REST_ANGLE 2

#define IS_MOVING_DEBOUNCE_CONSTANT 10

using std::chrono::duration_cast;
using std::chrono::milliseconds;
using std::chrono::seconds;

enum State
{
    INITIAL = 1,
    WAIT,
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
                                                                                             _deviceManager(deviceManager)
{
    _alarm = _deviceManager->GetAlarm();
    _copCoord.x = 0;
    _copCoord.y = 0;
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
    _currentDatetime = std::to_string(_deviceManager->GetTimeSinceEpoch());

    // Ces calls prennent beaucoup trop de CPU quand les capteurs ne sont pas connecté
    // UpdateSensor(DEVICES::alarmSensor, _deviceManager->IsAlarmConnected());
    // UpdateSensor(DEVICES::fixedImu, _deviceManager->IsFixedImuConnected());
    // UpdateSensor(DEVICES::mobileImu, _deviceManager->IsMobileImuConnected());
    // UpdateSensor(DEVICES::motionSensor, _deviceManager->IsMotionSensorConnected());

    if (_mosquittoBroker->CalibPressureMatRequired())
    {
        _deviceManager->CalibratePressureMat();
        _mosquittoBroker->SendIsPressureMatCalib(true, _currentDatetime);
    }

    if (_mosquittoBroker->CalibIMURequired())
    {
        _deviceManager->CalibrateIMU();
        _mosquittoBroker->SendIsIMUCalib(true, _currentDatetime);
    }

    _prevIsSomeoneThere = _isSomeoneThere;

    _deviceManager->Update();
    _isSomeoneThere = _deviceManager->IsSomeoneThere();
    _copCoord = _deviceManager->GetCenterOfPressure();
    _currentChairAngle = _deviceManager->GetBackSeatAngle();
    bool prevIsMoving = _isMoving;
    _isMoving = _deviceManager->GetIsMoving();
    _isChairInclined = _deviceManager->IsChairInclined();

#ifdef DEBUG_PRINT
    //printf("getDateTime = %s\n", _currentDatetime.c_str());
    printf("\n");
    printf("isSomeoneThere = %i\n", _isSomeoneThere);
    printf("getCenterOfPressure x = %f, y = %f\n", _copCoord.x, _copCoord.y);
    printf("_currentChairAngle = %i\n", _currentChairAngle);
    printf("_isMoving = %i\n", _isMoving);
    printf("_isChairInclined = %i\n", _isChairInclined);
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
            // printf("getXAcceleration (vibration) = %f\n", acceleration);
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
        // TODO valider que c'est le bon _state
        _state = State::INITIAL;
        printf("Something new for _requiredBackRestAngle = %i\n", _requiredBackRestAngle);
    }
    if (_mosquittoBroker->IsRequiredPeriodNew())
    {
        _requiredPeriod = _mosquittoBroker->GetRequiredPeriod();
        _secondsCounter = 0;
        // TODO valider que c'est le bon _state
        _state = State::INITIAL;
        printf("Something new for _requiredPeriod = %i\n", _requiredPeriod);
    }
    if (_mosquittoBroker->IsRequiredDurationNew())
    {
        _requiredDuration = _mosquittoBroker->GetRequiredDuration();
        _secondsCounter = 0;
        // TODO valider que c'est le bon _state
        _state = State::INITIAL;
        printf("Something new for _requiredDuration = %i\n", _requiredDuration);
    }
    if (_mosquittoBroker->IsWifiChanged())
    {
        NetworkManager::ChangeNetwork(_mosquittoBroker->GetWifiInformation());
        _isWifiChanged = true;
        _wifiChangedTimer.Reset();
    }
    _deviceManager->GetAlarm()->DeactivateVibration(_mosquittoBroker->IsVibrationDeactivated());
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
    printf("State INIT\t_secondsCounter: %i\n", _secondsCounter);

    if (++_secondsCounter >= REQUIRED_SITTING_TIME)
    {
        _state = State::WAIT;
        _secondsCounter = 0;
    }
}

void ChairManager::CheckIfBackRestIsRequired()
{
    printf("State WAIT\t_secondsCounter: %i\n", _secondsCounter);

    if (++_secondsCounter >= _requiredPeriod)
    {
        if (!_isMoving && !_isChairInclined)
        {
            _alarm->StopBlinkGreenAlarm();
            if (!_alarm->IsRedAlarmOn())
            {
                _alarm->TurnOnRedAlarmThread().detach();
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

    if (_failedTiltTimer.Elapsed() > FAILED_TILT_TIME.count()) // || check si le dismiss à été pesé ici)
    {
        _state = State::WAIT;
        printf("Failed bascule\n");
        _mosquittoBroker->SendTiltInfo(TiltInfo::FAIL, _currentDatetime);
    }

    if (_currentChairAngle > (_requiredBackRestAngle - DELTA_ANGLE_THRESHOLD))
    {
        _alarm->TurnOnGreenAlarm();
        _state = State::STAY;
    }
}

void ChairManager::CheckIfRequiredBackSeatAngleIsMaintained()
{
    printf("State STAY\n");

    if (_currentChairAngle > (_requiredBackRestAngle - DELTA_ANGLE_THRESHOLD))
    {
        _secondsCounter++;

        printf("State STAY\t_secondsCounter: %i\n", _secondsCounter);

        if (_secondsCounter >= _requiredDuration)
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
        _state = State::WAIT;
        printf("Bascule pas assez longue\n");
        _mosquittoBroker->SendTiltInfo(TiltInfo::TOO_SHORT, _currentDatetime);
    }
}

void ChairManager::CheckIfBackSeatIsBackToInitialPosition()
{
    _secondsCounter++;

    printf("State DESCEND\t_secondsCounter: %i\n", _secondsCounter);

    if (_currentChairAngle < (_requiredBackRestAngle - DELTA_ANGLE_THRESHOLD))
    {
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
