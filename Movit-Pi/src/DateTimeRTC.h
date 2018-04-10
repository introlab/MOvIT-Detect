#ifndef DATETIME_RTC_H
#define DATETIME_RTC_H

#include "MCP79410.h"

class DateTimeRTC
{
  public:
    void SetCurrentDateTime();
    void SetDefaultDateTime();
    int GetTimeSinceEpoch();

    static DateTimeRTC *getInstance()
    {
        static DateTimeRTC instance;
        return &instance;
    }

  private:
    DateTimeRTC();
    MCP79410 _mcp79410;
};

#endif
