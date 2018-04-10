#include "DateTimeRTC.h"

#include <Utils.h>
#include <stdio.h>
#include <time.h>
#include <stdlib.h>

DateTimeRTC::DateTimeRTC() { }

void DateTimeRTC::SetDefaultDateTime()
{
    _mcp79410.setDefaultDateTime();
}

void DateTimeRTC::SetCurrentDateTime()
{
	time_t rawtime;
	struct tm * ptm;

	time(&rawtime);
	ptm = gmtime(&rawtime);

	unsigned char dt[DATE_TIME_SIZE] = {
		DECToBCD(ptm->tm_sec),
		DECToBCD(ptm->tm_min),
		DECToBCD(ptm->tm_hour),
		0x00,
		DECToBCD(ptm->tm_mday),
		DECToBCD(ptm->tm_mon),
		DECToBCD(ptm->tm_year % 100),
	};

	time_t timeSinceEpoch = mktime(ptm);

    _mcp79410.setDateTime(dt);
}

int DateTimeRTC::GetTimeSinceEpoch()
{
	const int numberOfYearsToAdd = 100;
	unsigned char datetime[DATE_TIME_SIZE] = { 0, 0, 0, 0, 0, 0, 0 };

	_mcp79410.getDateTime(datetime);
	
	struct tm t = { 0 };

	t.tm_year = BCDToDEC(datetime[6]) + numberOfYearsToAdd;
	t.tm_mon = BCDToDEC(datetime[5]);
	t.tm_mday = BCDToDEC(datetime[4]);
	t.tm_hour = BCDToDEC(datetime[2]);
	t.tm_min = BCDToDEC(datetime[1]);
	t.tm_sec = BCDToDEC(datetime[0]);

	time_t timeSinceEpoch = mktime(&t);

    return (int)timeSinceEpoch;
}