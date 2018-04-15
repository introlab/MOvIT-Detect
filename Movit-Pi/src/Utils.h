#ifndef UTILS_H
#define UTILS_H

#include <stdint.h>
#include <math.h>

#define DEBUG_PRINT //Debug trace

#define ONES_MASK 0x0F
#define TENS_MASK 0xF0

const double radiansToDegrees = 180.0 / M_PI;

uint8_t BCDToDEC(const uint8_t &value);
uint8_t DECToBCD(const uint8_t &value);
uint8_t BCDAdd(const uint8_t &BCDlvalue, const uint8_t &DECrvalue);
uint8_t BCDSubstract(const uint8_t &BCDlvalue, const uint8_t &DECrvalue);

void SleepForMicroSeconds(uint32_t microseconds);
void SleepForMilliSeconds(uint32_t milliseconds);

//TODO maybe implement the following
//bool BCDGreaterThan(const uint8_t lvalue, const uint8_t rvalue);

#endif //UTILS_H
