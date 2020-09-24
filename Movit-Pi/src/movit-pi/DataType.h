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

struct SensorData {
    long time = 0;

    bool tofConnected = false;
    uint16_t tofRange = 0;

    bool flowConnected = false;
    int16_t flowTravelX = 0;
    int16_t flowTravelY = 0;

    bool alarmConnected = false;
    bool alarmRedLedOn = false;
    bool alarmRedLedBlink = false;
    bool alarmGreenLedOn = false;
    bool alarmGreenLedBlink = false;
    bool alarmAlternatingLedBlink= false;
    bool alarmDCMotorOn = false;
    bool alarmButtonPressed = false;

    bool matConnected = false;
    bool matCalibrated = false;
    float matThreshold = 0.0;
    uint16_t matData[9] = {0};

    bool mIMUConnected = false;
    bool mIMUCalibrated = false;
    double mIMUAccX = 0.0f;
    double mIMUAccY = 0.0f;
    double mIMUAccZ = 0.0f;
    double mIMUGyroX = 0.0f;
    double mIMUGyroY = 0.0f;
    double mIMUGyroZ = 0.0f;

    float fIMUConnected = false;
    bool fIMUCalibrated = false;
    double fIMUAccX = 0.0f;
    double fIMUAccY = 0.0f;
    double fIMUAccZ = 0.0f;
    double fIMUGyroX = 0.0f;
    double fIMUGyroY = 0.0f;
    double fIMUGyroZ = 0.0f;
};

struct ChairState {
    long time = 0;

    //En déplacement et la distance parcouru
    bool isMoving = false;
    uint32_t lastDistance = 0;

    //L'utilisateur est assis, on peut calculer le CG
    bool isSeated = false;                                                                          
    Coord_t centerOfGravity = {0.0f, 0.0f};                                                         
    Coord_t centerOfGravityPerQuadrant[4] = {{0.0f, 0.0f}, {0.0f, 0.0f}, {0.0f, 0.0f}, {0.0f, 0.0f}};

    float mIMUAngle = 0;
    float fIMUAngle = 0;
    float seatAngle = 0;

    bool button = false;
};
