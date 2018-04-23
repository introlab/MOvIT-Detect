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
    // Pour usage éventuel (un jour, ce code sera décommenté, pray for it)
    // std::clock_t start;
    // double duration;

    bool done = false;

    if (argc > 1 && std::string(argv[1]) == "-t")
    {
        while (!done)
        {
            done = devicemgr->TestDevices();
        }
    }
    else
    {
        while (!done)
        {
            chairmgr.UpdateDevices();
            chairmgr.ReadFromServer();
            chairmgr.CheckNotification();

            // TODO: trouver une facon d'attendre vraiment EXACTEMENT 1 sec
            usleep(EXECUTION_PERIOD);
        }
    }

    delete mosquittoBroker;
    return 0;
}
