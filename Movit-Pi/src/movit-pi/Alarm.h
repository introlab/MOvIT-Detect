#ifndef ALARM_H
#define ALARM_H

#include "Sensor.h"
#include "PCA9536.h"
#include <thread>
#include <mutex>

#define DEFAULT_BLINK_FREQUENCY 10

class Alarm : public Sensor
{
  public:
    Alarm();
    Alarm(double blinkFrequency);
    virtual ~Alarm();

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

    bool ButtonPressed() { 
      _pca9536.SetMode(PUSH_BUTTON, IO_INPUT);
      _pca9536.SetPolarity(PUSH_BUTTON, IO_INVERTED);
      return !GetPinState(PUSH_BUTTON); 
    }

    void TurnOnBlinkLedsAlarm();
    void TurnOnBlinkRedAlarm();
    void TurnOnGreenAlarm();
    void TurnOnBlinkGreenAlarm();
    void TurnOnAlternatingAlarm();

    void TurnOffBlinkGreenAlarm();
    void TurnOffBlinkRedAlarm();
    void TurnOffBlinkLedsAlarm();
    void TurnOffAlternatingAlarm();

    bool IsRedAlarmOn() { return _isRedAlarmOn; }
    bool IsGreenAlarmOn() {return _isGreenAlarmOn;}
    bool IsBlinkLedsAlarmOn() { return _isBlinkLedsAlarmOn; }
    bool IsBlinkRedAlarmOn() {return _isBlinkRedAlarmOn;}
    bool IsBlinkGreenAlarmOn() { return _isBlinkGreenAlarmOn; }
    bool IsAlternatingAlarmOn() { return _isAlternatingAlarmOn; }

    void TurnOnBlinkRedAlarmThread();
    void TurnOnBlinkLedsAlarmThread();
    void TurnOnBlinkGreenAlarmThread();
    void TurnOnAlternatingBlinkAlarmThread();

  private:
    PCA9536 _pca9536;

    bool _isGreenAlarmOn = false;
    bool _isRedAlarmOn = false;
    bool _isBlinkRedAlarmOn = false;
    bool _isBlinkLedsAlarmOn = false;
    bool _isBlinkGreenAlarmOn = false;
    bool _isAlternatingAlarmOn = false;

    std::thread blinkRedLedThread;
    std::thread blinkLedsThread;
    std::thread blinkGreenLedThread;
    std::thread alternatingLedThread;

    bool _deactivateVibration = true;
    bool _deactivateBlinking = false;

    double _blinkFrequency;
    std::recursive_mutex _mtx;

    uint8_t GetPinState(pin_t pin);
};

#endif // ALARM_H
