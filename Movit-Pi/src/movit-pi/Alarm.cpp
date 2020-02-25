#include "Alarm.h"
#include "Utils.h"
#include "SysTime.h"


#include <stdio.h>
#include <unistd.h>



Alarm::Alarm()
{
    Alarm(DEFAULT_BLINK_FREQUENCY);
}

Alarm::Alarm(double blinkFrequency) : _blinkFrequency(blinkFrequency)
{
}

bool Alarm::Initialize()
{
     std::lock_guard<std::recursive_mutex> lock(_mtx);
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
    std::lock_guard<std::recursive_mutex> lock(_mtx);
    return _pca9536.isConnected();
}

uint8_t Alarm::GetPinState(pin_t pin)
{
    std::lock_guard<std::recursive_mutex> lock(_mtx);
    return _pca9536.GetState(pin);
}

void Alarm::TurnOnDCMotor()
{
    std::lock_guard<std::recursive_mutex> lock(_mtx);
    _pca9536.SetMode(DC_MOTOR, IO_OUTPUT);
    if (GetPinState(DC_MOTOR) == IO_LOW)
    {
        _pca9536.SetState(DC_MOTOR, IO_HIGH);
    }
}

void Alarm::TurnOffDCMotor()
{   
    std::lock_guard<std::recursive_mutex> lock(_mtx);
    _pca9536.SetMode(DC_MOTOR, IO_OUTPUT);
    if (GetPinState(DC_MOTOR) == IO_HIGH)
    {
        _pca9536.SetState(DC_MOTOR, IO_LOW);
    }
    _pca9536.SetMode(DC_MOTOR, IO_INPUT);
}

void Alarm::TurnOnRedLed()
{
    std::lock_guard<std::recursive_mutex> lock(_mtx);
    _isRedAlarmOn = true;
    _pca9536.SetState(RED_LED, IO_HIGH);
}

void Alarm::TurnOffRedLed()
{
    std::lock_guard<std::recursive_mutex> lock(_mtx);
    _isRedAlarmOn = false;
    _pca9536.SetState(RED_LED, IO_LOW);
}

void Alarm::TurnOnGreenLed()
{
    std::lock_guard<std::recursive_mutex> lock(_mtx);
    _isGreenAlarmOn = true;
    _pca9536.SetState(GREEN_LED, IO_HIGH);
}

void Alarm::TurnOffGreenLed()
{
    std::lock_guard<std::recursive_mutex> lock(_mtx);
    _isGreenAlarmOn = false;
    _pca9536.SetState(GREEN_LED, IO_LOW);
}

void Alarm::TurnOffAlarm()
{
    printf("Alarm::TurnOffAlarm\n");
    TurnOffBlinkRedAlarm();
    TurnOffBlinkLedsAlarm();
    TurnOffBlinkGreenAlarm();
    TurnOffAlternatingAlarm();
    TurnOffRedLed();
    TurnOffGreenLed();
    TurnOffDCMotor();
}

void Alarm::TurnOnAlternatingAlarm() {
    _mtx.lock();
    _isAlternatingAlarmOn = true;
    _isGreenAlarmOn = false;
    _isRedAlarmOn = false;
    _mtx.unlock();

    TurnOffRedLed();
    TurnOffGreenLed();

    if (!_deactivateVibration)
    {
        TurnOnDCMotor();
    }
    int counter = 0;

    while (_isAlternatingAlarmOn)
    {   counter++;

        _mtx.lock();
        if(counter % 2 == 0) {

            _pca9536.SetState(GREEN_LED, IO_HIGH);
            _pca9536.SetState(RED_LED, IO_LOW);
        } else {
            _pca9536.SetState(GREEN_LED, IO_LOW);
            _pca9536.SetState(RED_LED, IO_HIGH);
        }
        _mtx.unlock();
        sleep_for_milliseconds((1 / _blinkFrequency) * SECONDS_TO_MILLISECONDS);
    }

    TurnOffRedLed();
    TurnOffGreenLed();
}

void Alarm::TurnOffAlternatingAlarm() {
    std::lock_guard<std::recursive_mutex> lock(_mtx);
    _isAlternatingAlarmOn = false;
}

void Alarm::TurnOnGreenAlarm()
{
    TurnOnGreenLed();
}

void Alarm::TurnOnBlinkLedsAlarm()
{
    _mtx.lock();
    _isBlinkLedsAlarmOn = true;
    _isRedAlarmOn = false;
    _isGreenAlarmOn = false;
    _mtx.unlock();

    if (!_deactivateVibration)
    {
        TurnOnDCMotor();
    }


    while (_isBlinkLedsAlarmOn)
    {
        _mtx.lock();
        _pca9536.ToggleState(RED_LED);
        _pca9536.ToggleState(GREEN_LED);
        _mtx.unlock();
        sleep_for_milliseconds((1 / _blinkFrequency) * SECONDS_TO_MILLISECONDS);
    }

    TurnOffRedLed();
    TurnOffGreenLed();
}

void Alarm::TurnOnBlinkRedAlarm()
{
    _mtx.lock();
    _isBlinkRedAlarmOn = true;
    _isRedAlarmOn = false;
    _mtx.unlock();

    if (!_deactivateVibration)
    {
        TurnOnDCMotor();
    }

    TurnOnRedLed();
    while (_isBlinkRedAlarmOn)
    {
        if (!_deactivateBlinking)
        {
            _mtx.lock();
            _pca9536.ToggleState(RED_LED);
            _mtx.unlock();
        }
        sleep_for_milliseconds((1 / _blinkFrequency) * SECONDS_TO_MILLISECONDS);
    }

    TurnOffRedLed();
}

void Alarm::TurnOnBlinkGreenAlarm()
{
    _mtx.lock();
    _isBlinkGreenAlarmOn = true;    
    _isGreenAlarmOn = false;
    _mtx.unlock();

    while (_isBlinkGreenAlarmOn)
    {
        _mtx.lock();
        _pca9536.ToggleState(GREEN_LED);
        _mtx.unlock();
        sleep_for_milliseconds((1 / _blinkFrequency) * SECONDS_TO_MILLISECONDS);
    }

    TurnOffGreenLed();
}

void Alarm::TurnOffBlinkLedsAlarm()
{
    std::lock_guard<std::recursive_mutex> lock(_mtx);
    _isBlinkLedsAlarmOn = false;
}

void Alarm::TurnOffBlinkRedAlarm()
{
    std::lock_guard<std::recursive_mutex> lock(_mtx);
    _isBlinkRedAlarmOn = false;
}

void Alarm::TurnOffBlinkGreenAlarm()
{
    std::lock_guard<std::recursive_mutex> lock(_mtx);
    _isBlinkGreenAlarmOn = false;
}

void Alarm::TurnOnBlinkRedAlarmThread()
{
    if(_isBlinkRedAlarmOn) {
        return;
    }
    printf("Alarm::TurnOnBlinkRedAlarmThread()\n");
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
    printf("Alarm::TurnOnBlinkLedsAlarmThread()\n");
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
    printf("Alarm::TurnOnBlinkGreenAlarmThread()\n");
    blinkGreenLedThread = std::thread([=] {
        TurnOnBlinkGreenAlarm();
    });
    blinkGreenLedThread.detach();
}

void Alarm::TurnOnAlternatingBlinkAlarmThread() {
    if(_isAlternatingAlarmOn) {
        return;
    }
    printf("Alarm::TurnOnAlternatingBlinkAlarmThread()\n");
    alternatingLedThread = std::thread([=] {
        TurnOnAlternatingAlarm();
    });
    alternatingLedThread.detach();
}
