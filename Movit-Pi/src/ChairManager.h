#ifndef CHAIR_MANAGER_H
#define CHAIR_MANAGER_H

#include "MosquittoBroker.h"
#include "Utils.h"
#include "Timer.h"
#include "DeviceManager.h"

#include <string>
#include <unistd.h>
#include <chrono>

class ChairManager
{
  public:
    ChairManager(MosquittoBroker *mosquittoBroker, DeviceManager *deviceManager);
    ~ChairManager();

    inline bool TestPattern() { return _deviceManager->TestDevices(); }

    void SendSensorsStatus();
    void UpdateDevices();
    void ReadFromServer();
    void CheckNotification();
    void SetVibrationsActivated(bool isVibrationsActivated);
    std::thread ReadVibrationsThread();

  private:

    static constexpr auto CENTER_OF_PRESSURE_EMISSION_PERIOD = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::minutes(5));
    static constexpr auto CHAIR_ANGLE_EMISSION_PERIOD = std::chrono::seconds(1);

    Alarm *_alarm;
    MosquittoBroker *_mosquittoBroker;
    DeviceManager *_deviceManager;

    uint32_t _secondsCounter = 0;
    uint8_t _state = 0;

    int _currentChairAngle = 0;
    int _prevChairAngle = 0;
    Coord_t _copCoord;
    std::string _currentDatetime = "";

    bool _isSomeoneThere = false;
    bool _prevIsSomeoneThere = false;
    bool _isMoving = false;
    bool _isChairInclined = false;

    bool _overrideNotificationPattern = false;
    bool _setAlarmOn = false;
    bool _isVibrationsActivated = true;

    int _requiredBackRestAngle = 0;
    uint32_t _requiredPeriod = 0;
    uint32_t _requiredDuration = 0;

    Timer _centerOfPressureTimer;
    Timer _chairAngleTimer;
    Timer _keepAliveTimer;

    void CheckIfUserHasBeenSittingForRequiredTime();
    void CheckIfBackRestIsRequired();
    void CheckIfRequiredBackSeatAngleIsReached();
    void CheckIfRequiredBackSeatAngleIsMaintained();
    void CheckIfBackSeatIsBackToInitialPosition();
    void OverrideNotificationPattern();
    void ReadVibrations();
};

#endif // CHAIR_MANAGER_H
