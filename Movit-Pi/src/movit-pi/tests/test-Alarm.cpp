#include "Alarm.h"
#include "I2Cdev.h"
#include <unistd.h>
#include <iostream>

using namespace std;

int main(int argc, char* argv[])
{

    Alarm alarm;

    alarm.Initialize();

    int count = 0;

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

        //0.1 sec sleep
        usleep(100000);

	if (count++ / 10 % 2 == 0)
		alarm.TurnOnDCMotor();
	else
		alarm.TurnOffDCMotor();

	if (alarm.ButtonPressed())
	   cout << "Button Pressed" << endl;

    }

    return 0;
}
