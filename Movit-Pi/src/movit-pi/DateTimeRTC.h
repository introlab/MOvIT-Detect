#ifndef DATETIME_RTC_H
#define DATETIME_RTC_H

#include "MCP79410.h"
#include <thread>

class DateTimeRTC
{
  public:
    std::thread SetCurrentDateTimeThread();
    void SetCurrentDateTime();
    void SetDefaultDateTime();
    int GetTimeSinceEpoch();

    static DateTimeRTC *GetInstance()
    {
        static DateTimeRTC instance;
        return &instance;
    }

  private:
    //Singleton
    DateTimeRTC();
    DateTimeRTC(DateTimeRTC const &);    // Don't Implement.
    void operator=(DateTimeRTC const &); // Don't implement.

    int BCDDateTimeToEpoch(uint8_t *dateTime);
    void EpochToBCDDateTime(int epoch, uint8_t *bcdDateTime);

    int GetSystemCurrentTime();
    int GetRTCCurrentTime();
    void SetCurrentDateTimeIfConnected();

    bool _isDatetimeSet = false;
    MCP79410 _mcp79410;
};

#endif // DATETIME_RTC_H
