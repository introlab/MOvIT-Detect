#ifndef DATETIME_RTC_H
#define DATETIME_RTC_H

#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <unistd.h>
#include <ctime>
#include <iomanip>
#include <sstream>

class DateTimeRTC
{
  public:
    static long GetTimeSinceEpoch();

  private:
    DateTimeRTC() {};
};

#endif // DATETIME_RTC_H
