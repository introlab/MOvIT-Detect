#include <signal.h>
#include <stdlib.h>
#include <stdio.h>
#include <string>
#include <unistd.h>
#include <ctime>
#include <chrono>
#include "MosquittoBroker.h"
#include "DeviceManager.h"
#include "ChairManager.h"
#include "Utils.h"

using std::string;
using std::chrono::duration;
using std::chrono::milliseconds;

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

    auto start = std::chrono::system_clock::now();
    auto end = std::chrono::system_clock::now();
    auto period = milliseconds(1000);

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
            start = std::chrono::system_clock::now();

            chairmgr.UpdateDevices();
            chairmgr.ReadFromServer();
            chairmgr.CheckNotification();

            end = std::chrono::system_clock::now();
            auto elapse_time = std::chrono::duration_cast<milliseconds>(end - start);

            sleep_for_milliseconds(period.count() - elapse_time.count());
        }
    }

    delete mosquittoBroker;
    return 0;
}
