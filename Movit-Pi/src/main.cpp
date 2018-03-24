#include <string>
#include <stdio.h>
#include <unistd.h>
#include <ctime>
#include "mosquitto_broker/mosquitto_broker.h"
#include "DeviceManager.h"
#include "ChairManager.h"

using std::string;

#define EXECUTION_PERIOD 1000000

int main()
{
    I2Cdev::initialize();

    MosquittoBroker *mosquittoBroker = new MosquittoBroker("embedded");
    DeviceManager *devicemgr = DeviceManager::getInstance();
    ChairManager chairmgr(mosquittoBroker, devicemgr);

    devicemgr->InitializeDevices();

    std::clock_t start;
    double duration;

    // uint32_t secondsCounter = 0;

    // int angle = 0;
    // int prevangle = 0;
    uint8_t state = 1;
    bool done = false;
    while (!done)
    {
        // Test thing
        // done = chairmgr.testPattern();

        // Real thing
        chairmgr.UpdateDevices();
        chairmgr.ReadFromServer();
        chairmgr.CheckNotification(state);

        //TODO trouver une facon d'attendre vraiment EXACTEMENT 1 sec
        usleep(EXECUTION_PERIOD);

        // char inSerialChar = getchar();

        // if (inSerialChar == 'q')
        //     done = true;
    }

    delete mosquittoBroker;
    return 0;
}
