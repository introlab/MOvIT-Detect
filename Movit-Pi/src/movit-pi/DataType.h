#pragma once

#include <stdint.h>

#define NUMBER_OF_AXIS 3
#define PRESSURE_SENSOR_COUNT 9

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

struct notifications_settings_t
{
    bool isLedBlinkingEnabled = true;
    bool isVibrationEnabled = true;
    float snoozeTime = 10.0f; //in minutes
};

struct sensor_state_t
{
    bool notificationModuleValid = false;
    bool fixedAccelerometerValid = false;
    bool mobileAccelerometerValid = false;
    bool pressureMatValid = false;
};

struct tilt_settings_t
{
    int requiredBackRestAngle = 0;
    uint32_t requiredPeriod = 0;
    uint32_t requiredDuration = 0;
};
