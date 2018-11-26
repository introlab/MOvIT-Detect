#include "DateTimeRTC.h"
#include "NetworkManager.h"
#include "Utils.h"
#include "SysTime.h"

#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <unistd.h>

const uint32_t sleepTime = 5;

DateTimeRTC::DateTimeRTC() {}

void DateTimeRTC::SetDefaultDateTime()
{
    _mcp79410.SetDefaultDateTime();
}

std::thread DateTimeRTC::SetCurrentDateTimeThread()
{
    return std::thread([=] { SetCurrentDateTimeIfConnected(); });
}

void DateTimeRTC::SetCurrentDateTimeIfConnected()
{
    while (!_isDatetimeSet)
    {
        if (GetRTCCurrentTime() >= GetSystemCurrentTime())
        {
            _isDatetimeSet = true;
        }
        else if (NetworkManager::IsConnected())
        {
            SetCurrentDateTime();
            _isDatetimeSet = true;
        }
        sleep_for_seconds(sleepTime);
    }
}

void DateTimeRTC::SetCurrentDateTime()
{
    uint8_t datetime[DATE_TIME_SIZE];
    int epoch = GetSystemCurrentTime();
    EpochToBCDDateTime(epoch, datetime);
    _mcp79410.SetDateTime(datetime);
}

int DateTimeRTC::GetTimeSinceEpoch()
{
    if (!_isDatetimeSet)
    {
        printf("Warning: The dateTime is not set.\n");
        return 0;
    }
    return GetRTCCurrentTime();
}

int DateTimeRTC::GetRTCCurrentTime()
{
    uint8_t bcdDatetime[DATE_TIME_SIZE];
    _mcp79410.GetDateTime(bcdDatetime);
    return BCDDateTimeToEpoch(bcdDatetime);
}

int DateTimeRTC::GetSystemCurrentTime()
{
    time_t rawtime;
    struct tm *ptm;

    time(&rawtime);
    ptm = gmtime(&rawtime);
    time_t timeSinceEpoch = mktime(ptm);

    return static_cast<int>(timeSinceEpoch);
}

int DateTimeRTC::BCDDateTimeToEpoch(uint8_t *bcdDateTime)
{
    const int numberOfYearsToAdd = 100;
    struct tm t = {0};

    t.tm_year = BCDToDEC(bcdDateTime[6]) + numberOfYearsToAdd;
    t.tm_mon = BCDToDEC(bcdDateTime[5]) - 1; //We need to remove one month since the RTC returns months from 1-12 and struct Tm want months from 0-11.
    t.tm_mday = BCDToDEC(bcdDateTime[4]);
    t.tm_hour = BCDToDEC(bcdDateTime[2]);
    t.tm_min = BCDToDEC(bcdDateTime[1]);
    t.tm_sec = BCDToDEC(bcdDateTime[0]);

    time_t timeSinceEpoch = mktime(&t);

    return static_cast<int>(timeSinceEpoch);
}

void DateTimeRTC::EpochToBCDDateTime(int epoch, uint8_t *bcdDateTime)
{
    time_t rawtime = static_cast<time_t>(epoch);
    struct tm *ptm;
    ptm = gmtime(&rawtime);

    bcdDateTime[0] = DECToBCD(ptm->tm_sec);
    bcdDateTime[1] = DECToBCD(ptm->tm_min);
    bcdDateTime[2] = DECToBCD(ptm->tm_hour);
    bcdDateTime[3] = 0x00;
    bcdDateTime[4] = DECToBCD(ptm->tm_mday);
    bcdDateTime[5] = DECToBCD(ptm->tm_mon + 1); //We need to add one month since struct Tm returns months from 0-11 and RTC want months from 1-12.
    bcdDateTime[6] = DECToBCD(ptm->tm_year % 100);
}
