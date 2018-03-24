#ifndef DATETIME_RTC_H
#define DATETIME_RTC_H

#include "MCP79410.h"
#include <string>

class DateTimeRTC
{
  public:
    void setDateTime(unsigned char dt[]);
    void setDefaultDateTime();

    void getDateTime(unsigned char dt[]);
    std::string getFormattedDateTime();

    // Singleton
    static DateTimeRTC *getInstance()
    {
        static DateTimeRTC instance;
        return &instance;
    }

  private:
    //Singleton
    DateTimeRTC();
    DateTimeRTC(DateTimeRTC const &);    // Don't Implement.
    void operator=(DateTimeRTC const &); // Don't implement

    void formatDateTimeString();

    MCP79410 _mcp79410;

    std::string _date;
    std::string _time;
    std::string _dateTime;
    unsigned char _rawDateTime[DATE_TIME_SIZE] = {0, 0, 0, 0, 0, 0, 0}; //{s, m, h, w, d, date, month, year}

    std::string _year;
    std::string _month;
    std::string _day;
    std::string _hour;
    std::string _minute;
    std::string _second;
};

#endif // DATETIME_RTC_H
