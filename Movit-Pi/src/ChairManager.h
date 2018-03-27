#ifndef CHAIR_MANAGER
#define CHAIR_MANAGER

#include "mosquitto_broker/mosquitto_broker.h"
#include "DeviceManager.h"
#include <string>
#include <unistd.h>
#include "Utils.h"

class ChairManager
{
  public:
    ChairManager(MosquittoBroker *mosquittoBroker, DeviceManager *devicemgr);

    bool TestPattern() { return _devicemgr->testDevices(); }

    void UpdateDevices();
    void ReadFromServer();
    void CheckNotification();

  private:
    MosquittoBroker *_mosquittoBroker;
    DeviceManager *_devicemgr;

    uint32_t _secondsCounter = 0;
    uint8_t _state = 0;

    int _currentChairAngle = 0;
    int _prevChairAngle = 0;
    Coord_t _copCoord;
    std::string _currentDatetime = "";
    bool _isSomeoneThere = false;

    bool _overrideNotificationPattern = false;

    bool _setAlarmOn = false;
    uint32_t _requiredBackRestAngle = 0;
    uint32_t _requiredPeriod = 0;
    uint32_t _requiredDuration = 0;
};

#endif //CHAIR_MANAGER