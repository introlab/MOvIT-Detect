#ifndef DATETIME_RTC_H
#define DATETIME_RTC_H

#include "MCP79410.h"
#include <thread>

class DateTimeRTC
{
  public:
    std::thread SetCurrentDateTimeIfConnectedThread();
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
	void SetCurrentDateTimeIfConnected();

	bool _isDatetimeSet = false;
    MCP79410 _mcp79410;
};

#endif
