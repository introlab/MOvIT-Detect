#include "MCP79410.h"
#include "Utils.h"

//For more information see: http://ww1.microchip.com/downloads/en/DeviceDoc/20002266H.pdf section 5.3

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
    I2Cdev::WriteByte(_devAddr, ADDR_SEC, DEFAULT_SEC);
    I2Cdev::WriteByte(_devAddr, ADDR_MIN, DEFAULT_MIN);
    I2Cdev::WriteByte(_devAddr, ADDR_HOUR, DEFAULT_HOUR);
    I2Cdev::WriteByte(_devAddr, ADDR_DAY, DEFAULT_DAY);
    I2Cdev::WriteByte(_devAddr, ADDR_DATE, DEFAULT_DATE);
    I2Cdev::WriteByte(_devAddr, ADDR_MNTH, DEFAULT_MNTH);
    I2Cdev::WriteByte(_devAddr, ADDR_YEAR, DEFAULT_YEAR);
    I2Cdev::WriteByte(_devAddr, ADDR_SEC, ENABLE_OSCILLATOR);
}

void MCP79410::SetDateTime(unsigned char dt[])
{
    I2Cdev::WriteByte(_devAddr, ADDR_SEC, DISABLE_OSCILLATOR);        //STOP MCP79410
    I2Cdev::WriteByte(_devAddr, ADDR_MIN, dt[1]);                     //MINUTE=20
    I2Cdev::WriteByte(_devAddr, ADDR_HOUR, dt[2] & 0x3f);             //HOUR=dt[2], forcing 24h format
    I2Cdev::WriteByte(_devAddr, ADDR_DAY, dt[3] | 0x0F);              //DAY=dt[3] AND VBAT=1
    I2Cdev::WriteByte(_devAddr, ADDR_DATE, dt[4]);                    //DATE=dt[4]
    I2Cdev::WriteByte(_devAddr, ADDR_MNTH, dt[5] & 0x1F);             //MONTH=dt[5] bloquing LPYR
    I2Cdev::WriteByte(_devAddr, ADDR_YEAR, dt[6]);                    //YEAR=dt[6]
    I2Cdev::WriteByte(_devAddr, ADDR_SEC, ENABLE_OSCILLATOR | dt[0]); //START  MCP79410, SECOND=dt[0];
}

void MCP79410::GetDateTime(unsigned char dt[])
{
    I2Cdev::ReadByte(_devAddr, ADDR_SEC, _buffer);
    dt[0] = *_buffer & 0xff >> (8 - _validBits[0]);
    I2Cdev::ReadByte(_devAddr, ADDR_MIN, _buffer);
    dt[1] = *_buffer & 0xff >> (8 - _validBits[1]);
    I2Cdev::ReadByte(_devAddr, ADDR_HOUR, _buffer);
    dt[2] = *_buffer & 0xff >> (8 - _validBits[2]);
    I2Cdev::ReadByte(_devAddr, ADDR_DATE, _buffer);
    dt[4] = *_buffer & 0xff >> (8 - _validBits[4]);
    I2Cdev::ReadByte(_devAddr, ADDR_MNTH, _buffer);
    dt[5] = *_buffer & 0xff >> (8 - _validBits[5]);
    I2Cdev::ReadByte(_devAddr, ADDR_YEAR, _buffer);
    dt[6] = *_buffer & 0xff >> (8 - _validBits[6]);
}
