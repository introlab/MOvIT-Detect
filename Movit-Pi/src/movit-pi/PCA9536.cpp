/*==============================================================================================================*

    @file     PCA9536.cpp
    @author   Nadav Matalon
    @license  MIT (c) 2016 Nadav Matalon

    PCA9536 Driver (4-Channel GPIO I2C Expander)

    Ver. 1.0.0 - First release (24.10.16)

 *===============================================================================================================*
    LICENSE
 *===============================================================================================================*

    The MIT License (MIT)
    Copyright (c) 2016 Nadav Matalon

    Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated
    documentation files (the "Software"), to deal in the Software without restriction, including without
    limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of
    the Software, and to permit persons to whom the Software is furnished to do so, subject to the following
    conditions:

    The above copyright notice and this permission notice shall be included in all copies or substantial
    portions of the Software.

    THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT
    LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
    IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
    WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE
    SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

 *==============================================================================================================*/

#include "PCA9536.h"
#include "I2Cdev.h"
#include <stdlib.h>
#include <stdio.h>

/*==============================================================================================================*
    CONSTRUCTOR
 *==============================================================================================================*/
PCA9536::PCA9536()
{
}

/*==============================================================================================================*
    DESTRUCTOR
 *==============================================================================================================*/
PCA9536::~PCA9536() {}

/*==============================================================================================================*
    GET MODE (0 = OUTPUT / 1 = INPUT)
 *==============================================================================================================*/
uint8_t PCA9536::GetMode(pin_t pin)
{
    std::lock_guard<std::recursive_mutex> lock(_mtx);
    return GetPin(pin, REG_CONFIG);
}

bool PCA9536::isConnected() {
    std::lock_guard<std::recursive_mutex> lock(_mtx);
    uint8_t regData = 0;
    uint8_t response = I2Cdev::ReadByte(DEV_ADDR, 0x02, &regData);
    uint8_t response2 = I2Cdev::ReadByte(DEV_ADDR, 0x03, &regData);
    //printf("PCA9536 isConnected %i, %i \n", response, response2);
    return response == 1 && response2 == 1;
}

/*==============================================================================================================*
    GET STATE (0 = LOW / 1 = HIGH)
 *==============================================================================================================*/
uint8_t PCA9536::GetState(pin_t pin)
{
    std::lock_guard<std::recursive_mutex> lock(_mtx);
    return GetPin(pin, GetMode(pin) ? REG_INPUT : REG_OUTPUT);
}

/*==============================================================================================================*
    GET POLARITY: INPUT PINS ONLY (0 = NON-INVERTED / 1 = INVERTED)
 *==============================================================================================================*/
uint8_t PCA9536::GetPolarity(pin_t pin)
{
    std::lock_guard<std::recursive_mutex> lock(_mtx);
    return GetPin(pin, REG_POLARITY);
}

/*==============================================================================================================*
    SET MODE
 *==============================================================================================================*/

void PCA9536::SetMode(pin_t pin, mode_t newMode)
{
    std::lock_guard<std::recursive_mutex> lock(_mtx);
    // PARAMS: IO0 / IO1 / IO2 / IO3
    SetPin(pin, REG_CONFIG, newMode); //         IO_INPUT / IO_OUTPUT
}

/*==============================================================================================================*
    SET MODE : ALL PINS
 *==============================================================================================================*/
void PCA9536::SetMode(mode_t newMode)
{
    std::lock_guard<std::recursive_mutex> lock(_mtx);
    // PARAMS: IO_INPUT / IO_OUTPUT
    SetReg(REG_CONFIG, newMode ? ALL_INPUT : ALL_OUTPUT);
}

/*==============================================================================================================*
    SET STATE (OUTPUT PINS ONLY)
 *==============================================================================================================*/
void PCA9536::SetState(pin_t pin, state_t newState)
{
    std::lock_guard<std::recursive_mutex> lock(_mtx);
    // PARAMS: IO0 / IO1 / IO2 / IO3
    SetPin(pin, REG_OUTPUT, newState); //         IO_LOW / IO_HIGH
}

/*==============================================================================================================*
    SET STATE : ALL PINS (OUTPUT PINS ONLY)
 *==============================================================================================================*/
void PCA9536::SetState(state_t newState)
{
    std::lock_guard<std::recursive_mutex> lock(_mtx);
    // PARAMS: IO_LOW / IO_HIGH
    SetReg(REG_OUTPUT, newState ? ALL_HIGH : ALL_LOW);
}

/*==============================================================================================================*
    TOGGLE STATE (OUTPUT PINS ONLY)
 *==============================================================================================================*/
void PCA9536::ToggleState(pin_t pin)
{
    std::lock_guard<std::recursive_mutex> lock(_mtx);
    SetReg(REG_OUTPUT, GetReg(REG_OUTPUT) ^ (1 << pin));
}

/*==============================================================================================================*
    TOGGLE STATE : ALL PINS (OUTPUT PINS ONLY)
 *==============================================================================================================*/
void PCA9536::ToggleState()
{
    std::lock_guard<std::recursive_mutex> lock(_mtx);
    SetReg(REG_OUTPUT, ~GetReg(REG_OUTPUT));
}

/*==============================================================================================================*
    SET POLARITY (INPUT PINS ONLY)
 *==============================================================================================================*/
void PCA9536::SetPolarity(pin_t pin, polarity_t newPolarity)
{
    std::lock_guard<std::recursive_mutex> lock(_mtx);
    // PARAMS: IO0 / IO1 / IO2 / IO3
    SetPin(pin, REG_POLARITY, newPolarity); //         IO_NON_INVERTED / IO_INVERTED
}

/*==============================================================================================================*
    SET POLARITY : ALL PINS (INPUT PINS ONLY)
 *==============================================================================================================*/
void PCA9536::SetPolarity(polarity_t newPolarity)
{
    std::lock_guard<std::recursive_mutex> lock(_mtx);
    // PARAMS: IO_NON_INVERTED / IO_INVERTED
    uint8_t polarityVals, polarityMask, polarityNew;
    polarityVals = GetReg(REG_POLARITY);
    polarityMask = GetReg(REG_CONFIG);
    polarityNew = newPolarity ? ALL_INVERTED : ALL_NON_INVERTED;
    SetReg(REG_POLARITY, (polarityVals & ~polarityMask) | (polarityNew & polarityMask));
}

/*==============================================================================================================*
    RESET
 *==============================================================================================================*/
void PCA9536::Reset()
{
    std::lock_guard<std::recursive_mutex> lock(_mtx);
    SetMode(IO_INPUT);
    SetState(IO_HIGH);
    SetPolarity(IO_NON_INVERTED);
}

/*==============================================================================================================*
    GET REGISTER DATA
 *==============================================================================================================*/
uint8_t PCA9536::GetReg(reg_ptr_t regPtr)
{
    std::lock_guard<std::recursive_mutex> lock(_mtx);
    uint8_t regData = 0;
    I2Cdev::ReadByte(DEV_ADDR, regPtr, &regData);
    return regData;
}

/*==============================================================================================================*
    GET PIN DATA
 *==============================================================================================================*/
uint8_t PCA9536::GetPin(pin_t pin, reg_ptr_t regPtr)
{
    std::lock_guard<std::recursive_mutex> lock(_mtx);
    return (GetReg(regPtr) >> pin) & 1U;
}

/*==============================================================================================================*
    SET REGISTER DATA
 *==============================================================================================================*/
void PCA9536::SetReg(reg_ptr_t regPtr, uint8_t newSetting)
{
    std::lock_guard<std::recursive_mutex> lock(_mtx);
    if (regPtr > 0)
    {
        if (!I2Cdev::WriteByte(DEV_ADDR, regPtr, newSetting))
        {
            printf("Error writing register: %2.2x val: %2.2x",regPtr, newSetting);
        }
    }
}

/*==============================================================================================================*
    SET PIN DATA
 *==============================================================================================================*/
void PCA9536::SetPin(pin_t pin, reg_ptr_t regPtr, uint8_t newSetting)
{
    std::lock_guard<std::recursive_mutex> lock(_mtx);
    uint8_t newReg = GetReg(regPtr);
    //printf("PCA9536::SetPin register output pin: %i, state : %2.2x \n", pin, newReg);
    if (newSetting)
    {
        newReg |= 1U << pin;
    }
    else
    {
        newReg &= ~(1U << pin);
    }
    SetReg(regPtr, newReg);
}
