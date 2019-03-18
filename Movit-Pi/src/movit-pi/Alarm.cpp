#include "Alarm.h"
#include "Utils.h"
#include "SysTime.h"

#include <mutex>
#include <stdio.h>
#include <unistd.h>

std::mutex mtx;

Alarm::Alarm()
{
    Alarm(DEFAULT_BLINK_FREQUENCY);
}

Alarm::Alarm(double blinkFrequency) : _blinkFrequency(blinkFrequency)
{
}

bool Alarm::Initialize()
{

    _pca9536.SetMode(DC_MOTOR, IO_INPUT);
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
    return _pca9536.isConnected();
}

uint8_t Alarm::GetPinState(pin_t pin)
{
    return _pca9536.GetState(pin);
}

void Alarm::TurnOnDCMotor()
{
    _pca9536.SetMode(DC_MOTOR, IO_OUTPUT);
    if (GetPinState(DC_MOTOR) == IO_LOW)
    {
        _pca9536.SetState(DC_MOTOR, IO_HIGH);
    }
}

void Alarm::TurnOffDCMotor()
{   
    _pca9536.SetMode(DC_MOTOR, IO_OUTPUT);
    if (GetPinState(DC_MOTOR) == IO_HIGH)
    {
        _pca9536.SetState(DC_MOTOR, IO_LOW);
    }
    _pca9536.SetMode(DC_MOTOR, IO_INPUT);
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
    TurnOffAlternatingAlarm();
    TurnOffRedLed();
    TurnOffGreenLed();
    TurnOffDCMotor();
}

void Alarm::TurnOnAlternatingAlarm() {
    _isAlternatingAlarmOn = true;

    if (!_deactivateVibration)
    {
        TurnOnDCMotor();
    }
    int counter = 0;

    while (_isAlternatingAlarmOn)
    {   counter++;

        if(counter % 2 == 0) {
            _pca9536.SetState(GREEN_LED, IO_HIGH);
            _pca9536.SetState(RED_LED, IO_LOW);
        } else {
            _pca9536.SetState(GREEN_LED, IO_LOW);
            _pca9536.SetState(RED_LED, IO_HIGH);
        }
        sleep_for_milliseconds((1 / _blinkFrequency) * SECONDS_TO_MILLISECONDS);
    }

    TurnOffRedLed();
    TurnOffGreenLed();
}

void Alarm::TurnOffAlternatingAlarm() {
    _isAlternatingAlarmOn = false;
}

void Alarm::TurnOnGreenAlarm()
{
    TurnOnGreenLed();
}

void Alarm::TurnOnBlinkLedsAlarm()
{
    _isBlinkLedsAlarmOn = true;

    if (!_deactivateVibration)
    {
        TurnOnDCMotor();
    }


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
}

void Alarm::TurnOnBlinkGreenAlarm()
{

    _isBlinkGreenAlarmOn = true;

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
    blinkGreenLedThread.detach();
}

void Alarm::TurnOnAlternatingBlinkAlarmThread() {
    if(_isAlternatingAlarmOn) {
        return;
    }

    alternatingLedThread = std::thread([=] {
        TurnOnAlternatingAlarm();
    });
    alternatingLedThread.detach();
}