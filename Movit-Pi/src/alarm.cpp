#include "alarm.h"

#include <stdio.h>
#include <unistd.h>

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
	// Sorry Ben j'ai pas le choix de l'enlever
	// Alarm::Initialize();
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
	int count = 0;
	TurnOffDCMotor();
	TurnOffRedLed();
	TurnOnGreenLed();

	while (GetPinState(PUSH_BUTTON) && (count++ <= (int)(_blinkFrequency * _blinkDuration)))
	{
		_pca9536.toggleState(RED_LED);
		_pca9536.toggleState(GREEN_LED);
		usleep(_blinkFrequency * 1000 * 1000);
	}

	TurnOffRedLed();
	TurnOffGreenLed();
}

void Alarm::TurnOnRedAlarm()
{
	int count = 0;
	TurnOnDCMotor();
	TurnOnRedLed();
	TurnOffGreenLed();

	while (GetPinState(PUSH_BUTTON) && (count++ <= (int)(_blinkFrequency * _blinkDuration)))
	{
		_pca9536.toggleState(RED_LED);
		usleep(_blinkFrequency * 1000 * 1000);
	}

	TurnOffRedLed();
	TurnOffDCMotor();
}