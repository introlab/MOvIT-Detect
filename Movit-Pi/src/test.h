#ifndef _TEST_H_
#define _TEST_H_

#include "alarm.h"
#include "forceSensor.h"
#include "forcePlate.h"
#include "DateTimeRTC.h"


bool program_test(Alarm &alarm, DateTimeRTC *datetimeRTC,  BackSeatAngleTracker &imu, uint16_t* max11611Data, forceSensor &sensorMatrix, forcePlate &globalForcePlate);

void printStuff();

#endif /* _TEST_H_ */
