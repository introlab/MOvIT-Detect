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
    ~Alarm();

    bool Initialize();
    bool IsConnected();

    void DeactivateVibration(bool state) { _deactivateVibration = state; }
    void DeactivateLedBlinking(bool state) { _deactivateBlinking = state; }

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

    void TurnOffBlinkGreenAlarm();
    void TurnOffBlinkRedAlarm();
    void TurnOffBlinkLedsAlarm();

    bool IsRedAlarmOn() { return _isBlinkRedAlarmOn; }
    bool IsBlinkLedsAlarmOn() { return _isBlinkLedsAlarmOn; }
    bool IsBlinkGreenAlarmOn() { return _isBlinkGreenAlarmOn; }

    void TurnOnBlinkRedAlarmThread();
    void TurnOnBlinkLedsAlarmThread();
    void TurnOnBlinkGreenAlarmThread();

  private:
    PCA9536 _pca9536;

    bool _isBlinkRedAlarmOn = false;
    bool _isBlinkLedsAlarmOn = false;
    bool _isBlinkGreenAlarmOn = false;

    std::thread blinkRedLedThread;
    std::thread blinkLedsThread;
    std::thread blinkGreenLedThread;

    bool _deactivateVibration = true;
    bool _deactivateBlinking = false;

    double _blinkFrequency;

    uint8_t GetPinState(pin_t pin);
};

#endif // ALARM_H
