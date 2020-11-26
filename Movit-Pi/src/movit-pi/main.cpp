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
#include "SysTime.h"
#include "FileManager.h"

using std::string;
using std::chrono::duration;
using std::chrono::milliseconds;

void exit_program_handler(int s)
{

    if (s == SIGINT)
    {
    	printf("SIGINT, exiting...\n");			
    	FileManager *fileManager = FileManager::GetInstance();
    	DeviceManager *deviceManager = DeviceManager::GetInstance(fileManager);
    	deviceManager->TurnOff();
    	exit(1);
    }
    else if (s == SIGALRM)
    {
	//Watchdog    
	printf("Watchdog timer, exiting\n");
	exit(2);
    }
    else 
    {
	printf("Unknown signal received: %i \n",s);
	exit(3);
    }
}

void printHeader() {
    printf(" __  __  ____      _____ _______     _____       _            _   \n");
    printf("|  \\/  |/ __ \\    |_   _|__   __|   |  __ \\     | |          | |  \n");
    printf("| \\  / | |  | |_   _| |    | |______| |  | | ___| |_ ___  ___| |_ \n");
    printf("| |\\/| | |  | \\ \\ / / |    | |______| |  | |/ _ \\ __/ _ \\/ __| __|\n");
    printf("| |  | | |__| |\\ V /| |_   | |      | |__| |  __/ ||  __/ (__| |_ \n");
    printf("|_|  |_|\\____/  \\_/_____|  |_|      |_____/ \\___|\\__\\___|\\___|\\__|\n\n");
}

int main(int argc, char *argv[])
{

    if (getuid()) {
        printf("Please run as root, exiting...\n");
        return 0;    
    }

    struct sigaction sigIntHandler;

    sigIntHandler.sa_handler = exit_program_handler;
    sigemptyset(&sigIntHandler.sa_mask);
    sigIntHandler.sa_flags = 0;
    
    std::chrono::high_resolution_clock::time_point start;
    std::chrono::high_resolution_clock::time_point stop;
    std::chrono::duration<double> elapsed;
    int sleepTime = 0;

    sigaction(SIGINT, &sigIntHandler, NULL);
    sigaction(SIGALRM, &sigIntHandler, NULL);

    MosquittoBroker mosquittoBroker("embedded");
    FileManager *fileManager = FileManager::GetInstance();
    DeviceManager *deviceManager = DeviceManager::GetInstance(fileManager);
    deviceManager->InitializeDevices();
    ChairManager chairManager(&mosquittoBroker, deviceManager);


    //Alarm testalarm;
    //testalarm.TurnOnRedLed();    
    //testalarm.TurnOnAlternatingBlinkAlarmThread();
    //system("clear");
    printHeader();

    while (1)
    {
	alarm(30); //Alarm in 30 secs
        start = std::chrono::high_resolution_clock::now();

	//printf("ReadFromServer()\n");
        //chairManager.ReadFromServer();

	printf("UpdateDevices()\n");
        chairManager.UpdateDevices();

	//printf("CheckNotification() \n"); //Does nothing?
        //chairManager.CheckNotification();

        stop = std::chrono::high_resolution_clock::now();

        elapsed = stop - start;
        sleepTime = ((1.00 * 1000000) - (elapsed.count() * 1000000));
        alarm(0); //Cancel alarm (watchdog)
 	    printf("------------------------------------------- Sleeping for %i ms \n",(sleepTime >= 0) ? sleepTime/1000: 0);
    	usleep((sleepTime >= 0) ? sleepTime : 0);
    }
    return 0;
}
