#include "Alarm.h"
#include "Utils.h"

#include <mutex>
#include <stdio.h>
#include <unistd.h>

std::mutex alarmMutex;

Alarm::Alarm()
{
	Alarm(DEFAULT_BLINK_DURATION, DEFAULT_BLINK_FREQUENCY);
}

Alarm::Alarm(int blinkDuration, double blinkFrequency)
{
	_blinkDuration = blinkDuration;
	_blinkFrequency = blinkFrequency;
}

void Alarm::Initialize()
{
	printf("PC9536 (Alarm) initializing ... ");
	_pca9536.setMode(DC_MOTOR, IO_OUTPUT);
	_pca9536.setMode(GREEN_LED, IO_OUTPUT);
	_pca9536.setMode(RED_LED, IO_OUTPUT);
	_pca9536.setState(IO_LOW);
	_pca9536.setMode(PUSH_BUTTON, IO_INPUT);
	_pca9536.setPolarity(PUSH_BUTTON, IO_INVERTED);

	if (_pca9536.getMode(DC_MOTOR) != IO_OUTPUT || _pca9536.getMode(GREEN_LED) != IO_OUTPUT || _pca9536.getMode(RED_LED) != IO_OUTPUT)
	{
		printf("FAIL\n");
	}
	else
	{
		printf("success\n");
	}
	TurnOffRedLed();
	TurnOffGreenLed();
	TurnOffDCMotor();
}

uint8_t Alarm::GetPinState(pin_t pin)
{
	return _pca9536.getState(pin);
}

void Alarm::SetBlinkDuration(int blinkDuration)
{
	_blinkDuration = blinkDuration;
}

void Alarm::SetBlinkFrequency(double blinkFrequency)
{
	_blinkFrequency = blinkFrequency;
}

void Alarm::TurnOnDCMotor()
{
	_pca9536.setState(DC_MOTOR, IO_HIGH);
}

void Alarm::TurnOffDCMotor()
{
	_pca9536.setState(DC_MOTOR, IO_LOW);
}

void Alarm::TurnOnRedLed()
{
	_pca9536.setState(RED_LED, IO_HIGH);
}

void Alarm::TurnOffRedLed()
{
	_pca9536.setState(RED_LED, IO_LOW);
}

void Alarm::TurnOnGreenLed()
{
	_pca9536.setState(GREEN_LED, IO_HIGH);
}

void Alarm::TurnOffGreenLed()
{
	_pca9536.setState(GREEN_LED, IO_LOW);
}

void Alarm::TurnOffAlarm()
{
	TurnOffRedLed();
	TurnOffGreenLed();
	TurnOffDCMotor();
}

void Alarm::TurnOnBlinkLedsAlarm()
{
    std::lock_guard<std::mutex> lock(alarmMutex);

	if (_isBlinkLedsAlarmOn)
	{
		return;
	}

	_isBlinkLedsAlarmOn = true;
	int count = 0;
	TurnOffDCMotor();
	TurnOffRedLed();
	TurnOnGreenLed();

	while (count++ <= (int)(_blinkFrequency * _blinkDuration))
	{
		_pca9536.toggleState(RED_LED);
		_pca9536.toggleState(GREEN_LED);
		usleep(_blinkFrequency * secondsToMicroseconds);
	}

	TurnOffRedLed();
	TurnOffGreenLed();
	_isBlinkLedsAlarmOn = false;
}

void Alarm::TurnOnRedAlarm()
{
    std::lock_guard<std::mutex> lock(alarmMutex);

	if (_isRedAlarmOn)
	{
		return;
	}

	_isRedAlarmOn = true;
	int count = 0;
	TurnOnDCMotor();
	TurnOnRedLed();
	TurnOffGreenLed();

	while (GetPinState(PUSH_BUTTON) && (count++ <= (int)(_blinkFrequency * _blinkDuration)))
	{
		_pca9536.toggleState(RED_LED);
		usleep(_blinkFrequency * secondsToMicroseconds);
	}

	TurnOnRedLed();
	TurnOffDCMotor();
	_isRedAlarmOn = false;
}

void Alarm::TurnOnGreenAlarm()
{
    std::lock_guard<std::mutex> lock(alarmMutex);

    TurnOnGreenLed();
    TurnOffRedLed();
    TurnOffDCMotor();
}

std::thread Alarm::TurnOnRedAlarmThread()
{
    return std::thread([=] { 
        TurnOnRedAlarm(); 
    });
}

std::thread Alarm::TurnOnBlinkLedsAlarmThread()
{
	return std::thread([=] { 
        TurnOnBlinkLedsAlarm(); 
    });
}
