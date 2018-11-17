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
#include "FileManager.h"

using std::string;
using std::chrono::duration;
using std::chrono::milliseconds;

void exit_program_handler(int s)
{
    FileManager *fileManager = FileManager::GetInstance();
    DeviceManager *deviceManager = DeviceManager::GetInstance(fileManager);
    deviceManager->TurnOff();
    exit(1);
}

int main(int argc, char *argv[])
{
    struct sigaction sigIntHandler;

    sigIntHandler.sa_handler = exit_program_handler;
    sigemptyset(&sigIntHandler.sa_mask);
    sigIntHandler.sa_flags = 0;

    sigaction(SIGINT, &sigIntHandler, NULL);

    MosquittoBroker mosquittoBroker("embedded");
    FileManager *fileManager = FileManager::GetInstance();
    DeviceManager *deviceManager = DeviceManager::GetInstance(fileManager);
    ChairManager chairManager(&mosquittoBroker, deviceManager);

    deviceManager->InitializeDevices();
    chairManager.SendSensorsState();

    auto start = std::chrono::system_clock::now();
    auto end = std::chrono::system_clock::now();
    auto period = milliseconds(1000);

    chairManager.ReadVibrationsThread().detach();

    while (true)
    {
        start = std::chrono::system_clock::now();

        chairManager.UpdateDevices();
        chairManager.ReadFromServer();
        chairManager.CheckNotification();

        end = std::chrono::system_clock::now();
        auto elapse_time = std::chrono::duration_cast<milliseconds>(end - start);

        if (elapse_time.count() >= period.count())
        {
            elapse_time = period;
        }

        sleep_for_milliseconds(period.count() - elapse_time.count());
    }

    chairManager.SetVibrationsActivated(false);
    return 0;
}
