#include "DateTimeRTC.h"

long DateTimeRTC::GetTimeSinceEpoch() {
   return time(0);
}

