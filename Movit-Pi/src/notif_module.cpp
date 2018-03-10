#include "notif_module.h"
#include <stdio.h>
#include <unistd.h>

Alarm::Alarm()
{
    Alarm(DEFAULT_BLINK_DURATION, DEFAULT_BLINK_FREQUENCY);
}

Alarm::Alarm(int blinkDuration, double blinkFrequency)
{
    _isRedLedEnabled = true;
    _isGreenLedEnabled = true;
    _isDCMotorEnabled = true;
    _blinkDuration = blinkDuration;
    _blinkFrequency = blinkFrequency;

    Alarm::initialize();
    SetPinState(DC_MOTOR, IO_LOW, _isDCMotorEnabled);
    SetPinState(RED_LED, IO_LOW, _isRedLedEnabled);
    SetPinState(GREEN_LED, IO_LOW, _isGreenLedEnabled);
}

void Alarm::initialize()
{
    printf("PCA9536 (Alarm) initializing ... ");
    _pca9536.reset();
    _pca9536.setMode(DC_MOTOR, IO_OUTPUT);
    _pca9536.setMode(GREEN_LED, IO_OUTPUT);
    _pca9536.setMode(RED_LED, IO_OUTPUT);
    _pca9536.setState(IO_LOW);
    _pca9536.setMode(PUSH_BUTTON, IO_INPUT);
    _pca9536.setPolarity(PUSH_BUTTON, IO_INVERTED);

    if (_pca9536.getMode(DC_MOTOR) != IO_OUTPUT || _pca9536.getMode(GREEN_LED) != IO_OUTPUT || _pca9536.getMode(RED_LED) != IO_OUTPUT)
    {
        printf("FAIL\n");
    }
    else
    {
        printf("success\n");
    }
}

void Alarm::SetBlinkDuration(int blinkDuration)
{
    _blinkDuration = blinkDuration;
}

void Alarm::SetBlinkFrequency(double blinkFrequency)
{
    _blinkFrequency = blinkFrequency;
}

void Alarm::SetIsRedLedEnabled(bool isRedLedEnabled)
{
    _isRedLedEnabled = isRedLedEnabled;
}

void Alarm::SetIsGreenLedEnabled(bool isGreenLedEnabled)
{
    _isGreenLedEnabled = isGreenLedEnabled;
}

void Alarm::SetIsDCMotorEnabled(bool isDCMotorEnabled)
{
    _isDCMotorEnabled = isDCMotorEnabled;
}

void Alarm::StartBuzzer()
{
    _pca9536.setState(DC_MOTOR, IO_HIGH);
}
void Alarm::StopBuzzer()
{
    _pca9536.setState(DC_MOTOR, IO_LOW);
}

void Alarm::SetPinState(pin_t pin, Pca9536::state_t state, bool isEnabled)
{
    if (isEnabled)
    {
        _pca9536.setState(pin, state);
    }
    else
    {
        _pca9536.setState(pin, IO_LOW);
    }
}

uint8_t Alarm::GetPinState(pin_t pin)
{
    return _pca9536.getState(pin);
}

void Alarm::TurnOffAlarm()
{
    SetPinState(GREEN_LED, IO_LOW, _isGreenLedEnabled);
    SetPinState(RED_LED, IO_LOW, _isRedLedEnabled);
    SetPinState(DC_MOTOR, IO_LOW, _isDCMotorEnabled);
}

void Alarm::TurnOnAlarm()
{
    int count = 0;
    SetPinState(DC_MOTOR, IO_HIGH, _isDCMotorEnabled);
    SetPinState(RED_LED, IO_HIGH, _isRedLedEnabled);
    SetPinState(GREEN_LED, IO_LOW, _isGreenLedEnabled);

    while (GetPinState(PUSH_BUTTON) && (count++ <= (int)(_blinkFrequency * _blinkDuration)))
    {
        ChangeRedLedState();
        ChangeGreenLedState();
        usleep(_blinkFrequency * 1000 * 1000);
    }

    TurnOffAlarm();
}

void Alarm::TurnOnBlinkLedsAlarm()
{
    SetIsDCMotorEnabled(false);
    TurnOnAlarm();
    SetIsDCMotorEnabled(true);
}

void Alarm::TurnOnRedAlarm()
{
    SetIsGreenLedEnabled(false);
    TurnOnAlarm();
    SetIsGreenLedEnabled(true);
}

void Alarm::ChangeRedLedState()
{
    if (!GetPinState(RED_LED))
    {
        SetPinState(RED_LED, IO_HIGH, _isRedLedEnabled);
    }
    else
    {
        SetPinState(RED_LED, IO_LOW, _isRedLedEnabled);
    }
}

void Alarm::ChangeGreenLedState()
{
    if (!GetPinState(GREEN_LED))
    {
        SetPinState(GREEN_LED, IO_HIGH, _isGreenLedEnabled);
    }
    else
    {
        SetPinState(GREEN_LED, IO_LOW, _isGreenLedEnabled);
    }
}
