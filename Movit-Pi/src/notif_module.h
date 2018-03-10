#ifndef _NOTIF_MODULE_H_
#define _NOTIF_MODULE_H_

#include "PCA9536.h"

#define DEFAULT_BLINK_DURATION 1000
#define DEFAULT_BLINK_FREQUENCY 0.1

class Alarm
{
  private:
	PCA9536 _pca9536;
	bool _isDCMotorEnabled;
	bool _isRedLedEnabled;
	bool _isGreenLedEnabled;
	double _blinkFrequency;
	int _blinkDuration;

  public:
	Alarm();
	Alarm(int blinkDuration, double blinkFrequency);

	void initialize();
	void SetBlinkDuration(int blinkDuraction);
	void SetBlinkFrequency(double blinkFrequency);
	void SetIsRedLedEnabled(bool isRedLedEnabled);
	void SetIsGreenLedEnabled(bool isGreenLedEnabled);
	void SetIsDCMotorEnabled(bool isMotorDCEnabled);
	void SetPinState(pin_t pin, Pca9536::state_t state, bool isEnabled);

	void StartBuzzer();
	void StopBuzzer();

	uint8_t GetPinState(pin_t pin);

	void ChangeRedLedState();
	void ChangeGreenLedState();
	void TurnOnAlarm();
	void TurnOffAlarm();
	void TurnOnBlinkLedsAlarm();
	void TurnOnRedAlarm();
};

#endif
