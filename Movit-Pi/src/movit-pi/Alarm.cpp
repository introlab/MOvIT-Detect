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
    _pca9536.SetMode(DC_MOTOR, IO_OUTPUT);
    _pca9536.SetMode(GREEN_LED, IO_OUTPUT);
    _pca9536.SetMode(RED_LED, IO_OUTPUT);
    _pca9536.SetState(IO_LOW);
    _pca9536.SetMode(PUSH_BUTTON, IO_INPUT);
    _pca9536.SetPolarity(PUSH_BUTTON, IO_INVERTED);

    if (!IsConnected())
    {
        return false;
    }

    TurnOffRedLed();
    TurnOffGreenLed();
    TurnOffDCMotor();
    return true;
}

Alarm::~Alarm() {
    TurnOffAlarm();
}

bool Alarm::IsConnected()
{
    return _pca9536.GetMode(DC_MOTOR) == IO_OUTPUT && _pca9536.GetMode(GREEN_LED) == IO_OUTPUT && _pca9536.GetMode(RED_LED) == IO_OUTPUT;
}

uint8_t Alarm::GetPinState(pin_t pin)
{
    return _pca9536.GetState(pin);
}

void Alarm::TurnOnDCMotor()
{
    //if (GetPinState(DC_MOTOR) == IO_LOW)
    //{
        _pca9536.SetState(DC_MOTOR, IO_HIGH);
    //}
}

void Alarm::TurnOffDCMotor()
{
    //if (GetPinState(DC_MOTOR) == IO_HIGH)
    //{
        _pca9536.SetState(DC_MOTOR, IO_LOW);
    //}
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
    TurnOffBlinkRedAlarm();
    TurnOffBlinkLedsAlarm();
    TurnOffBlinkGreenAlarm();
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

    //std::lock_guard<std::mutex> lock(alarmMutex);

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

    //std::lock_guard<std::mutex> lock(alarmMutex);

    if (!_deactivateVibration)
    {
        TurnOnDCMotor();
    }

    TurnOnRedLed();
    while (_isBlinkRedAlarmOn)
    {
        if (!_deactivateBlinking)
        {
            _pca9536.ToggleState(RED_LED);
        }
        sleep_for_milliseconds((1 / _blinkFrequency) * SECONDS_TO_MILLISECONDS);
    }

    TurnOffRedLed();
    TurnOffDCMotor();
}

void Alarm::TurnOnBlinkGreenAlarm()
{
    _isBlinkGreenAlarmOn = true;

    //std::lock_guard<std::mutex> lock(alarmMutex);

    while (_isBlinkGreenAlarmOn)
    {
        _pca9536.ToggleState(GREEN_LED);
        sleep_for_milliseconds((1 / _blinkFrequency) * SECONDS_TO_MILLISECONDS);
    }

    TurnOffGreenLed();
}

void Alarm::TurnOffBlinkLedsAlarm()
{
    _isBlinkLedsAlarmOn = false;
}

void Alarm::TurnOffBlinkRedAlarm()
{
    _isBlinkRedAlarmOn = false;
}

void Alarm::TurnOffBlinkGreenAlarm()
{
    _isBlinkGreenAlarmOn = false;
}

void Alarm::TurnOnBlinkRedAlarmThread()
{
    if(_isBlinkRedAlarmOn) {
        return;
    }

    blinkRedLedThread = std::thread([=] {
        TurnOnBlinkRedAlarm();
    });
    blinkRedLedThread.detach();
}

void Alarm::TurnOnBlinkLedsAlarmThread()
{   
    if(_isBlinkLedsAlarmOn) {
        return;
    }

    blinkLedsThread = std::thread([=] {
        TurnOnBlinkLedsAlarm();
    });
    blinkLedsThread.detach();
}

void Alarm::TurnOnBlinkGreenAlarmThread()
{
    if(_isBlinkGreenAlarmOn) {
        return;
    }

    blinkGreenLedThread = std::thread([=] {
        TurnOnBlinkGreenAlarm();
    });
    blinkRedLedThread.detach();
    
}
