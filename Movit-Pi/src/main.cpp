#include <signal.h>
#include <stdlib.h>
#include <stdio.h>
#include <string>
#include <unistd.h>
#include <ctime>
#include "MosquittoBroker/MosquittoBroker.h"
#include "DeviceManager.h"
#include "ChairManager.h"

using std::string;

#define EXECUTION_PERIOD 1000000

void exit_program_handler(int s) 
{
    DeviceManager *devicemgr = DeviceManager::GetInstance();
    devicemgr->TurnOff();
    exit(1);
}

int main(int argc, char *argv[])
{
    I2Cdev::Initialize();

    struct sigaction sigIntHandler;

    sigIntHandler.sa_handler = exit_program_handler;
    sigemptyset(&sigIntHandler.sa_mask);
    sigIntHandler.sa_flags = 0;

    sigaction(SIGINT, &sigIntHandler, NULL);

    MosquittoBroker *mosquittoBroker = new MosquittoBroker("embedded");
    DeviceManager *devicemgr = DeviceManager::GetInstance();
    ChairManager chairmgr(mosquittoBroker, devicemgr);

    devicemgr->InitializeDevices();
    // Pour usage éventuel
    // std::clock_t start;
    // double duration;

    if (argc > 1)
    {
        if (std::string(argv[1]) == "-t")
        {
            while (true)
            {
                devicemgr->TestDevices();
            }
        }
    }
    
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
