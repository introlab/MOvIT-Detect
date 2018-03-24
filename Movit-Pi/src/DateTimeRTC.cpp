#include "DateTimeRTC.h"

using std::string;

#define YEAR_LENGTH 4
#define MONTH_LENGTH 2
#define DAY_LENGTH 2
#define HOUR_LENGTH 2
#define MINUTE_LENGTH 2
#define SECOND_LENGTH 2
#define DATE_LENGTH YEAR_LENGTH + 1 + MONTH_LENGTH + 1 + DAY_LENGTH
#define TIME_LENGTH HOUR_LENGTH + 1 + MINUTE_LENGTH + 1 + SECOND_LENGTH
#define DATETIME_LENGTH DATE_LENGTH + TIME_LENGTH

#include <stdio.h>

DateTimeRTC::DateTimeRTC()
{
    _year = "";
    _month = "";
    _day = "";
    _hour = "";
    _minute = "";
    _second = "";
    _date = "";
    _time = "";
    _dateTime = "";
}

void DateTimeRTC::setDefaultDateTime()
{
    _mcp79410.setDefaultDateTime();
}

void DateTimeRTC::setDateTime(unsigned char dt[])
{
    _mcp79410.setDateTime(dt);
}

void DateTimeRTC::getDateTime(unsigned char dt[])
{
    _mcp79410.getDateTime(_rawDateTime);

    for (unsigned int i = 0; i < DATE_TIME_SIZE; i++)
    {
        dt[i] = _rawDateTime[i];
    }
}

string DateTimeRTC::getFormattedDateTime()
{
    unsigned char dummy[DATE_TIME_SIZE];
    getDateTime(dummy);
    formatDateTimeString();

    return _dateTime;
}


void DateTimeRTC::formatDateTimeString()
{
    //TODO ajouter des commentaires par rapport aux lignes suivante
    _year = "20";
    _year += std::to_string(int((_rawDateTime[6] & 0xF0) >> 4)) + std::to_string(int(_rawDateTime[6] & 0x0F));
    _month = std::to_string(int((_rawDateTime[5] & 0x10) >> 4)) + std::to_string(int(_rawDateTime[5] & 0x0F));
    _day = std::to_string(int((_rawDateTime[4] & 0xF0) >> 4)) + std::to_string(int(_rawDateTime[4] & 0x0F));
    _hour = std::to_string(int((_rawDateTime[2] & 0x30) >> 4)) + std::to_string(int(_rawDateTime[2] & 0x0F));
    _minute = std::to_string(int((_rawDateTime[1] & 0xF0) >> 4)) + std::to_string(int(_rawDateTime[1] & 0x0F));
    _second = std::to_string(int((_rawDateTime[0] & 0x30) >> 4)) + std::to_string(int(_rawDateTime[0] & 0x0F));

    _date = _year + "-" + _month + "-" + _day;
    _time = _hour + ":" + _minute + ":" + _second;
    _dateTime = _date + ":" + _time;
}