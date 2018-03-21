#ifndef _PROGRAM_H_
#define _PROGRAM_H_

#include <string>
#include "alarm.h"

using std::string;

#define DEBUG_SERIAL //Debug trace

#include "forceSensor.h"
#include "forcePlate.h"

bool program_loop(Alarm &alarm, BackSeatAngleTracker &imu, uint16_t* max11611Data, forceSensor &sensorMatrix, forcePlate &globalForcePlate);
string& getCmd();
void sendData(string& request, bool state, bool e);
void getData(uint16_t* max11611Data, forceSensor &sensorMatrix);
void formatDateTimeString();

#endif /* _PROGRAM_H_ */
