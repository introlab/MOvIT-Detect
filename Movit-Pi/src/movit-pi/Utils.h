#ifndef UTILS_H
#define UTILS_H

#include <string>
#include <stdint.h>
#include <math.h>

#define DEBUG_PRINT //Debug trace

#define ONES_MASK 0x0F
#define TENS_MASK 0xF0

enum AXIS { x, y, z };
enum DEVICES { alarmSensor, fixedImu, mobileImu, motionSensor, pressureMat };
const std::string FIXED_IMU_NAME = "fixedImu";
const std::string MOBILE_IMU_NAME = "mobileImu";
const double RADIANS_TO_DEGREES = 180.0 / M_PI;

uint8_t BCDToDEC(const uint8_t &value);
uint8_t DECToBCD(const uint8_t &value);
uint8_t BCDAdd(const uint8_t &BCDlvalue, const uint8_t &DECrvalue);
uint8_t BCDSubstract(const uint8_t &BCDlvalue, const uint8_t &DECrvalue);

#endif //UTILS_H
