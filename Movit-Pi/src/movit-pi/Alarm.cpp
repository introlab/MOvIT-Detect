#include "Alarm.h"
#include "Utils.h"
#include "SysTime.h"

#include <mutex>
#include <stdio.h>
#include <unistd.h>

std::mutex alarmMutex;

Alarm::Alarm()
{
    Alarm(DEFAULT_BLINK_FREQUENCY);
}

Alarm::Alarm(double blinkFrequency) : _blinkFrequency(blinkFrequency)
{
}

bool Alarm::Initialize()
{
    printf("PC9536 (Alarm) initializing ... ");
    _pca9536.SetMode(DC_MOTOR, IO_OUTPUT);
    _pca9536.SetMode(GREEN_LED, IO_OUTPUT);
    _pca9536.SetMode(RED_LED, IO_OUTPUT);
    _pca9536.SetState(IO_LOW);
    _pca9536.SetMode(PUSH_BUTTON, IO_INPUT);
    _pca9536.SetPolarity(PUSH_BUTTON, IO_INVERTED);

    if (!IsConnected())
    {
        printf("FAIL\n");
        return false;
    }

    printf("SUCCESS\n");
    TurnOffRedLed();
    TurnOffGreenLed();
    TurnOffDCMotor();
    return true;
}

bool Alarm::IsConnected()
{
    return _pca9536.GetMode(DC_MOTOR) == IO_OUTPUT
        && _pca9536.GetMode(GREEN_LED) == IO_OUTPUT
        && _pca9536.GetMode(RED_LED) == IO_OUTPUT;
}

uint8_t Alarm::GetPinState(pin_t pin)
{
    return _pca9536.GetState(pin);
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
    StopBlinkRedAlarm();
    StopBlinkLedsAlarm();
    StopBlinkGreenAlarm();
    TurnOffRedLed();
    TurnOffGreenLed();
    TurnOffDCMotor();
}

void Alarm::TurnOnGreenAlarm()
{
    std::lock_guard<std::mutex> lock(alarmMutex);

    TurnOnGreenLed();
}

void Alarm::TurnOnBlinkLedsAlarm()
{
    _isBlinkLedsAlarmOn = true;

    std::lock_guard<std::mutex> lock(alarmMutex);

    while (_isBlinkLedsAlarmOn)
    {
        _pca9536.ToggleState(RED_LED);
        _pca9536.ToggleState(GREEN_LED);
        sleep_for_milliseconds((1 / _blinkFrequency) * SECONDS_TO_MILLISECONDS);
    }

    TurnOffRedLed();
    TurnOffGreenLed();
}

void Alarm::TurnOnBlinkRedAlarm()
{
    _isBlinkRedAlarmOn = true;

    std::lock_guard<std::mutex> lock(alarmMutex);

    if (!_deactivateVibration)
    {
        TurnOnDCMotor();
    }

    while (_isBlinkRedAlarmOn)
    {
        _pca9536.ToggleState(RED_LED);
        sleep_for_milliseconds((1 / _blinkFrequency) * SECONDS_TO_MILLISECONDS);
    }

    TurnOffRedLed();
    TurnOffDCMotor();
}

void Alarm::TurnOnBlinkGreenAlarm()
{
    _isBlinkGreenAlarmOn = true;

    std::lock_guard<std::mutex> lock(alarmMutex);

    while (_isBlinkGreenAlarmOn)
    {
        _pca9536.ToggleState(GREEN_LED);
        sleep_for_milliseconds((1 / _blinkFrequency) * SECONDS_TO_MILLISECONDS);
    }

    TurnOffGreenLed();
}

void Alarm::StopBlinkLedsAlarm()
{
    _isBlinkLedsAlarmOn = false;
}

void Alarm::StopBlinkRedAlarm()
{
    _isBlinkRedAlarmOn = false;
}

void Alarm::StopBlinkGreenAlarm()
{
    _isBlinkGreenAlarmOn = false;
}

std::thread Alarm::TurnOnBlinkRedAlarmThread()
{
    return std::thread([=] {
        TurnOnBlinkRedAlarm();
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
