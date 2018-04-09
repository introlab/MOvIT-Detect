#include <string>
#include <stdio.h>
#include <unistd.h>
#include <ctime>
#include "MosquittoBroker/MosquittoBroker.h"
#include "DeviceManager.h"
#include "ChairManager.h"

using std::string;

#define EXECUTION_PERIOD 1000000

int main()
{
    I2Cdev::initialize();

    MosquittoBroker *mosquittoBroker = new MosquittoBroker("embedded");
    DeviceManager *devicemgr = DeviceManager::GetInstance();
    ChairManager chairmgr(mosquittoBroker, devicemgr);

    devicemgr->InitializeDevices();

    // Pour usage éventuel
    // std::clock_t start;
    // double duration;

    bool done = false;
    while (!done)
    {
        // Test thing
        // done = chairmgr.testPattern();

        // Real thing
        chairmgr.UpdateDevices();
        chairmgr.ReadFromServer();
        chairmgr.CheckNotification();

        //TODO trouver une facon d'attendre vraiment EXACTEMENT 1 sec
        usleep(EXECUTION_PERIOD);

        // char inSerialChar = getchar();

        // if (inSerialChar == 'q')
        //     done = true;
    }

    delete mosquittoBroker;
    return 0;
}
