#include "Alarm.h"
#include "I2Cdev.h"
#include <unistd.h>
#include <iostream>

using namespace std;

int main(int argc, char* argv[])
{

    Alarm alarm;

    alarm.Initialize();

    while(alarm.IsConnected())
    {
        //cout << "Alarm is connected." << endl;


//        alarm.TurnOnRedLed();
//        usleep(100000);
//        alarm.TurnOffRedLed();
//        usleep(100000);
//        alarm.TurnOnGreenLed();
//        usleep(100000);
//        alarm.TurnOffGreenLed();
//        alarm.TurnOnAlternatingAlarm();

        alarm.TurnOnAlternatingBlinkAlarmThread();

        alarm.TurnOffDCMotor();

        //1 sec sleep
        usleep(10000);
    }

    return 0;
}
