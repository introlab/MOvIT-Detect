#ifndef UTILS_H
#define UTILS_H

#include <string>
#include <stdint.h>
#include <math.h>

#define DEBUG_PRINT //Debug trace

#define ONES_MASK 0x0F
#define TENS_MASK 0xF0

enum AXIS { x, y, z };
const std::string FIXED_IMU_NAME = "fixedImu";
const std::string MOBILE_IMU_NAME = "mobileImu";
const double radiansToDegrees = 180.0 / M_PI;
const int secondsToMicroseconds = 1000000;

uint8_t BCDToDEC(const uint8_t &value);
uint8_t DECToBCD(const uint8_t &value);
uint8_t BCDAdd(const uint8_t &BCDlvalue, const uint8_t &DECrvalue);
uint8_t BCDSubstract(const uint8_t &BCDlvalue, const uint8_t &DECrvalue);

void sleep_for_microseconds(uint32_t microseconds);
void sleep_for_milliseconds(uint32_t milliseconds);

//TODO maybe implement the following
//bool BCDGreaterThan(const uint8_t lvalue, const uint8_t rvalue);

#endif //UTILS_H
