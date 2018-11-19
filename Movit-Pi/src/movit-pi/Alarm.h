#ifndef ALARM_H
#define ALARM_H

#include "Sensor.h"
#include "PCA9536.h"
#include <thread>

#define DEFAULT_BLINK_FREQUENCY 10

class Alarm : public Sensor
{
public:
  Alarm();
  Alarm(double blinkFrequency);
  ~Alarm() = default;

  bool Initialize();
  bool IsConnected();

  void DeactivateVibration(bool state) { _deactivateVibration = state; }

  void TurnOnDCMotor();
  void TurnOffDCMotor();
  void TurnOnRedLed();
  void TurnOffRedLed();
  void TurnOnGreenLed();
  void TurnOffGreenLed();
  void TurnOffAlarm();

  bool ButtonPressed() { return !GetPinState(PUSH_BUTTON); }

  void TurnOnBlinkLedsAlarm();
  void TurnOnBlinkRedAlarm();
  void TurnOnGreenAlarm();
  void TurnOnBlinkGreenAlarm();

  void StopBlinkGreenAlarm();
  void StopBlinkRedAlarm();
  void StopBlinkLedsAlarm();

  bool IsRedAlarmOn() { return _isBlinkRedAlarmOn; }
  bool IsBlinkLedsAlarmOn() { return _isBlinkLedsAlarmOn; }
  bool IsBlinkGreenAlarmOn() { return _isBlinkGreenAlarmOn; }

  std::thread TurnOnBlinkRedAlarmThread();
  std::thread TurnOnBlinkLedsAlarmThread();
  std::thread TurnOnBlinkGreenAlarmThread();

private:
  PCA9536 _pca9536;

  bool _isBlinkRedAlarmOn = false;
  bool _isBlinkLedsAlarmOn = false;
  bool _isBlinkGreenAlarmOn = false;

  bool _deactivateVibration = false;

  double _blinkFrequency;

  uint8_t GetPinState(pin_t pin);
};

#endif // ALARM_H
