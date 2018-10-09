#include "MotionSensor.h"
#include <math.h>
#include "Utils.h"

#define TIME_BETWEEN_READINGS 250 // In milliseconde
#define MINIMUM_WORKING_RANGE 80  //The sensor needs a minimum of 80 mm to the ground.

#define WHEELCHAIR_MOVING_THRESHOLD 2500
#define WHEELCHAIR_MOVING_TIMEOUT 3000 // In milliseconde
#define MOVING_AVG_WINDOW_SIZE 10

const char *FAIL_MESSAGE = "FAIL \n";
const char *SUCCESS_MESSAGE = "SUCCESS \n";

MotionSensor::MotionSensor() : _rangeAverage(MOVING_AVG_WINDOW_SIZE),
                               _deltaXAverage(MOVING_AVG_WINDOW_SIZE),
                               _deltaYAverage(MOVING_AVG_WINDOW_SIZE)
{
}

bool MotionSensor::Initialize()
{
    bool isRangeSensorInitialized = InitializeRangeSensor();
    bool isFlowSensorInitialized = InitializeOpticalFlowSensor();
    bool isInitialized = isRangeSensorInitialized && isFlowSensorInitialized && ValidDistanceToTheGround();

    if (isInitialized)
    {
        GetDeltaXYThread().detach();
        return true;
    }
    return false;
}

bool MotionSensor::InitializeRangeSensor()
{
    uint16_t const timeout = 500;         // In milliseconde
    uint32_t const timingBudget = 200000; // In milliseconde, high accuracy mode

    printf("VL53L0X (Range sensor) initializing ... ");
    if (!_rangeSensor.Initialize(false))
    {
        printf(FAIL_MESSAGE);
        return false;
    }
    _rangeSensor.SetTimeout(timeout);
    _rangeSensor.SetMeasurementTimingBudget(timingBudget);

    printf(SUCCESS_MESSAGE);
    return true;
}

bool MotionSensor::InitializeOpticalFlowSensor()
{
    printf("PMW3901 (Flow sensor) initializing ... ");

    if (!_opticalFLowSensor.Initialize())
    {
        printf(FAIL_MESSAGE);
        return false;
    }
    printf(SUCCESS_MESSAGE);
    return true;
}

std::thread MotionSensor::GetDeltaXYThread()
{
    return std::thread([=] { GetDeltaXY(); });
}

void MotionSensor::GetDeltaXY()
{
    while (true)
    {
        ReadRangeSensor();
        ReadFlowSensor();
        sleep_for_milliseconds(TIME_BETWEEN_READINGS);
    }
}

void MotionSensor::ReadRangeSensor()
{
    uint16_t range = _rangeSensor.ReadRangeSingleMillimeters();
    if (range != 8190)
    {
        _rangeAverage.AddSample(range);
    }
}

void MotionSensor::ReadFlowSensor()
{
    int16_t deltaX = 0;
    int16_t deltaY = 0;
    _opticalFLowSensor.ReadMotionCount(&deltaX, &deltaY);

    _deltaXAverage.AddSample(deltaX);
    _deltaYAverage.AddSample(deltaY);

    UpdateTravel();
}

void MotionSensor::UpdateTravel()
{
    int16_t deltaX = _deltaXAverage.GetAverage();
    int16_t deltaY = _deltaYAverage.GetAverage();
    double travelInPixels = sqrt((double)((deltaX * deltaX) + (deltaY * deltaY)));
    uint16_t travelInMillimeter = PixelsToMillimeter(travelInPixels);
    _isMovingTravel += travelInMillimeter;

#ifdef DEBUG_PRINT
    printf("Range: %f \n", GetAverageRange());
    printf("Travel in mm: %u \n", travelInMillimeter);
    printf("Is moving?: %s \n", travelInMillimeter ? "True" : "False");
    std::ofstream file;

    file.open("MotionSensor.txt", std::ofstream::out | std::ofstream::app);

    if (!file.is_open())
    {
        return;
    }
    file << deltaX << ";" << deltaY << ";" << GetAverageRange() << ";" << travelInMillimeter << ";" << GetIsMoving() << std::endl;
    file.close();
#endif
}

double MotionSensor::GetAverageRange()
{
    if (_rangeSensor.TimeoutOccurred())
    {
        printf("ERROR: Timeout occurred in range sensor \n");
    }
    return _rangeAverage.GetAverage();
}

bool MotionSensor::ValidDistanceToTheGround()
{
    printf("Validation of the minimum height of the flow sensor ... ");
    if (GetAverageRange() < MINIMUM_WORKING_RANGE)
    {
        printf(FAIL_MESSAGE);
        return false;
    }
    printf(SUCCESS_MESSAGE);
    return true;
}

bool MotionSensor::GetIsMoving()
{
    if (_timer.Elapsed() >= WHEELCHAIR_MOVING_TIMEOUT && _isMovingTravel < WHEELCHAIR_MOVING_THRESHOLD)
    {
        return false;
    }
    else if (_timer.Elapsed() < WHEELCHAIR_MOVING_TIMEOUT && _isMovingTravel >= WHEELCHAIR_MOVING_THRESHOLD)
    {
        _isMovingTravel = 0;
        _deltaXAverage.Reset();
        _deltaYAverage.Reset();
        _rangeAverage.Reset();
        _timer.Reset();
        return true;
    }
    return false;
}

uint16_t MotionSensor::PixelsToMillimeter(double pixels)
{
    // The camera has a field of view of 42 degrees or 0.733038285 rad.
    // The sensor is 30 pixels by 30 pixels
    // We assume that the sensor is perpendicular to the ground.
    // Arc Length = fov_in_rad * height_from_the_ground
    double const nbOfPixels = 30.0f;
    double const fov = 0.733038285f;
    return (uint32_t)((pixels * fov * GetAverageRange()) / nbOfPixels);
}
