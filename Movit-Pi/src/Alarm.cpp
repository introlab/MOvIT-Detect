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
    _pca9536.SetMode(DC_MOTOR, IO_OUTPUT);
    _pca9536.SetMode(GREEN_LED, IO_OUTPUT);
    _pca9536.SetMode(RED_LED, IO_OUTPUT);
    _pca9536.SetState(IO_LOW);
    _pca9536.SetMode(PUSH_BUTTON, IO_INPUT);
    _pca9536.SetPolarity(PUSH_BUTTON, IO_INVERTED);

    if (_pca9536.GetMode(DC_MOTOR) != IO_OUTPUT || _pca9536.GetMode(GREEN_LED) != IO_OUTPUT || _pca9536.GetMode(RED_LED) != IO_OUTPUT)
    {
        printf("FAIL\n");
    }
    else
    {
        printf("SUCCESS\n");
    }
    TurnOffRedLed();
    TurnOffGreenLed();
    TurnOffDCMotor();
}

uint8_t Alarm::GetPinState(pin_t pin)
{
    return _pca9536.GetState(pin);
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
    _pca9536.SetState(DC_MOTOR, IO_HIGH);
}

void Alarm::TurnOffDCMotor()
{
    _pca9536.SetState(DC_MOTOR, IO_LOW);
}

void Alarm::TurnOnRedLed()
{
    _pca9536.SetState(RED_LED, IO_HIGH);
}

void Alarm::TurnOffRedLed()
{
    _pca9536.SetState(RED_LED, IO_LOW);
}

void Alarm::TurnOnGreenLed()
{
    _pca9536.SetState(GREEN_LED, IO_HIGH);
}

void Alarm::TurnOffGreenLed()
{
    _pca9536.SetState(GREEN_LED, IO_LOW);
}

void Alarm::TurnOffAlarm()
{
    StopBlinkGreenAlarm();
    TurnOffRedLed();
    TurnOffGreenLed();
    TurnOffDCMotor();
}

void Alarm::StopBlinkGreenAlarm()
{
    _isBlinkGreenAlarmRequired = false;
}

void Alarm::TurnOnBlinkLedsAlarm()
{
    if (_isBlinkLedsAlarmOn)
    {
        return;
    }

    _isBlinkLedsAlarmOn = true;

    std::lock_guard<std::mutex> lock(alarmMutex);

    int count = 0;
    TurnOffDCMotor();
    TurnOffRedLed();
    TurnOnGreenLed();

    while (count++ <= (int)(_blinkFrequency * _blinkDuration))
    {
        _pca9536.ToggleState(RED_LED);
        _pca9536.ToggleState(GREEN_LED);
        sleep_for_microseconds(_blinkFrequency * secondsToMicroseconds);
    }

    TurnOffRedLed();
    TurnOffGreenLed();
    _isBlinkLedsAlarmOn = false;
}

void Alarm::TurnOnRedAlarm()
{
    if (_isRedAlarmOn)
    {
        return;
    }

    _isRedAlarmOn = true;

    std::lock_guard<std::mutex> lock(alarmMutex);

    int count = 0;
    TurnOnDCMotor();
    TurnOnRedLed();
    TurnOffGreenLed();

    while (GetPinState(PUSH_BUTTON) && (count++ <= (int)(_blinkFrequency * _blinkDuration)))
    {
        _pca9536.ToggleState(RED_LED);
        sleep_for_microseconds(_blinkFrequency * secondsToMicroseconds);
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

void Alarm::TurnOnBlinkGreenAlarm()
{
    if (_isBlinkGreenAlarmOn)
    {
        return;
    }

    _isBlinkGreenAlarmOn = true;
    _isBlinkGreenAlarmRequired = true;

    std::lock_guard<std::mutex> lock(alarmMutex);

    TurnOffDCMotor();
    TurnOffRedLed();
    TurnOnGreenLed();

    while (_isBlinkGreenAlarmRequired)
    {
        _pca9536.ToggleState(GREEN_LED);
        sleep_for_microseconds(_blinkFrequency * secondsToMicroseconds);
    }

    TurnOffGreenLed();

    _isBlinkGreenAlarmOn = false;
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

std::thread Alarm::TurnOnBlinkGreenAlarmThread()
{
    return std::thread([=] {
        TurnOnBlinkGreenAlarm();
    });
}
