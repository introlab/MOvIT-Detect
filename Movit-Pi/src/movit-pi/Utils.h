#ifndef UTILS_H
#define UTILS_H

#include <string>
#include <stdint.h>
#include <math.h>

#define DEBUG_PRINT //Debug trace

#define ONES_MASK 0x0F
#define TENS_MASK 0xF0
#define NUMBER_OF_AXIS 3
#define PRESSURE_SENSOR_COUNT 9

enum AXIS { x, y, z };
enum DEVICES { alarmSensor, fixedImu, mobileImu, motionSensor, plateSensor };
const std::string FIXED_IMU_NAME = "fixedImu";
const std::string MOBILE_IMU_NAME = "mobileImu";
const double RADIANS_TO_DEGREES = 180.0 / M_PI;

uint8_t BCDToDEC(const uint8_t &value);
uint8_t DECToBCD(const uint8_t &value);
uint8_t BCDAdd(const uint8_t &BCDlvalue, const uint8_t &DECrvalue);
uint8_t BCDSubstract(const uint8_t &BCDlvalue, const uint8_t &DECrvalue);


struct Coord_t
{
    float x;
    float y;
};

struct imu_offset_t
{
    int accelerometerOffsets[NUMBER_OF_AXIS] = {0, 0, 0};
    int gyroscopeOffsets[NUMBER_OF_AXIS] = {0, 0, 0};
};

struct pressure_mat_offset_t
{
    uint16_t analogOffset[PRESSURE_SENSOR_COUNT] = {0, 0, 0, 0, 0, 0, 0, 0, 0};
    uint32_t totalSensorMean = 0;
    float detectionThreshold = 0;
};

struct pressure_mat_data_t
{
    Coord_t centerOfPressure = {0.0f, 0.0f};
    Coord_t quadrantPressure[PRESSURE_SENSOR_COUNT] = {{0.0f, 0.0f}, {0.0f, 0.0f}, {0.0f, 0.0f}, {0.0f, 0.0f}};
};

#endif //UTILS_H
