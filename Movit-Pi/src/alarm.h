#ifndef _ALARM_H_
#define _ALARM_H_

#include "PCA9536.h"
#include <thread>

#define DEFAULT_BLINK_DURATION 1000
#define DEFAULT_BLINK_FREQUENCY 0.1

class Alarm
{
  private:
	PCA9536 _pca9536;
	double _blinkFrequency;
	int _blinkDuration;

	uint8_t GetPinState(pin_t pin);

  public:
	Alarm();
	Alarm(int blinkDuration, double blinkFrequency);

	void Initialize();
	void SetBlinkDuration(int blinkDuraction);
	void SetBlinkFrequency(double blinkFrequency);

	void TurnOnDCMotor();
	void TurnOffDCMotor();
	void TurnOnRedLed();
	void TurnOffRedLed();
	void TurnOnGreenLed();
	void TurnOffGreenLed();
	void TurnOnBlinkLedsAlarm();
	void TurnOnRedAlarm();
	void TurnOffAlarm();

	std::thread TurnOnRedAlarmThread();
	std::thread TurnOnBlinkLedsAlarmThread();
};

#endif
