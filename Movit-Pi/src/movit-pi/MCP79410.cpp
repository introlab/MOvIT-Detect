#include "MCP79410.h"
#include "Utils.h"
#include <unistd.h>

//For more information see: http://ww1.microchip.com/downloads/en/DeviceDoc/20002266H.pdf section 5.3
#define STOP_RTC_SLEEP_TIME 1000 //Min 100 us

#define ENABLE_OSCILLATOR 0x80
#define DISABLE_OSCILLATOR 0x00

#define DEFAULT_SEC 0x00  //Seconds = 0 and stop oscillator
#define DEFAULT_MIN 0x00  //Minutes = 0
#define DEFAULT_HOUR 0x40 //Hour = 0AM 12h format selected
#define DEFAULT_DAY 0x05  //Day = 1 VBATEN = 1
#define DEFAULT_DATE 0x11 //Day: Tens = 1 Ones =1
#define DEFAULT_MNTH 0x03 //Month: Tens = 0 Ones = 3
#define DEFAULT_YEAR 0x18 //Year: Tens = 1 Ones = 8

#define PM_BIT 0x20
#define HOUR_VALID_BITS 0x1F
#define HOUR_FORMAT_12H 0x40

MCP79410::MCP79410()
{
    _devAddr = ADDR_MCP79410;
}

void MCP79410::SetDefaultDateTime()
{
    I2Cdev::WriteByte(_devAddr, ADDR_SEC, DISABLE_OSCILLATOR);
    sleep_for_microseconds(STOP_RTC_SLEEP_TIME);
    I2Cdev::WriteByte(_devAddr, ADDR_SEC, DEFAULT_SEC);
    I2Cdev::WriteByte(_devAddr, ADDR_MIN, DEFAULT_MIN);
    I2Cdev::WriteByte(_devAddr, ADDR_HOUR, DEFAULT_HOUR);
    I2Cdev::WriteByte(_devAddr, ADDR_DAY, DEFAULT_DAY);
    I2Cdev::WriteByte(_devAddr, ADDR_DATE, DEFAULT_DATE);
    I2Cdev::WriteByte(_devAddr, ADDR_MNTH, DEFAULT_MNTH);
    I2Cdev::WriteByte(_devAddr, ADDR_YEAR, DEFAULT_YEAR);
    I2Cdev::WriteByte(_devAddr, ADDR_SEC, ENABLE_OSCILLATOR);
}

void MCP79410::SetDateTime(uint8_t dt[])
{
    SetLeapYearBit(dt);
    I2Cdev::WriteByte(_devAddr, ADDR_SEC, DISABLE_OSCILLATOR);        //STOP MCP79410
    sleep_for_microseconds(STOP_RTC_SLEEP_TIME);
    I2Cdev::WriteByte(_devAddr, ADDR_SEC, DISABLE_OSCILLATOR);        //STOP MCP79410
    I2Cdev::WriteByte(_devAddr, ADDR_MIN, dt[1]);                     //MINUTE=20
    I2Cdev::WriteByte(_devAddr, ADDR_HOUR, dt[2] & 0x3f);             //HOUR=dt[2], forcing 24h format
    I2Cdev::WriteByte(_devAddr, ADDR_DAY, dt[3] | 0x0F);              //DAY=dt[3] AND VBAT=1
    I2Cdev::WriteByte(_devAddr, ADDR_DATE, dt[4]);                    //DATE=dt[4]
    I2Cdev::WriteByte(_devAddr, ADDR_MNTH, dt[5]);                    //MONTH=dt[5]
    I2Cdev::WriteByte(_devAddr, ADDR_YEAR, dt[6]);                    //YEAR=dt[6]
    I2Cdev::WriteByte(_devAddr, ADDR_SEC, ENABLE_OSCILLATOR | dt[0]); //START  MCP79410, SECOND=dt[0];
}

void MCP79410::GetDateTime(uint8_t dt[])
{
    uint8_t buffer[DATE_TIME_SIZE];
    I2Cdev::ReadBytes(_devAddr, ADDR_SEC, DATE_TIME_SIZE, buffer);
    for (uint8_t i = 0; i < DATE_TIME_SIZE; i++)
    {
        dt[i] = buffer[i] & 0xFF >> (8 - _validBits[i]);
    }
}

void MCP79410::SetLeapYearBit(uint8_t dt[])
{
    const uint8_t leapYearBit = 5;
    uint16_t year = BCDToDEC(dt[6]) + 2000;

    if (IsALeapYear(year))
    {
        dt[5] |= 1 << leapYearBit;
    }
    else
    {
        dt[5] &= ~(1 << leapYearBit);
    }
}

bool MCP79410::IsALeapYear(uint16_t year)
{
    /* Check if the year is divisible by 4 or 
	is divisible by 400 */
    return ((year % 4 == 0 && year % 100 != 0) || (year % 400 == 0));
}
