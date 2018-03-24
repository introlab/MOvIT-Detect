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
    void CheckNotification(uint8_t &state);

  private:
    MosquittoBroker *_mosquittoBroker;
    DeviceManager *_devicemgr;

    uint32_t _secondsCounter = 0;

    int _currentChairAngle = 0;
    int _prevChairAngle = 0;
    Coord_t _copCoord;
    std::string _currentDatetime = "";
    bool _isSomeoneThere = false;

    bool setAlarmOn = false;
    uint32_t requiredBackRestAngle = 0;
    uint32_t requiredPeriod = 0;
    uint32_t requiredDuration = 0;
};

#endif //CHAIR_MANAGER