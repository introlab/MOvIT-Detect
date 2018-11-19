#include "MotionSensor.h"
#include <math.h>
#include "Utils.h"
#include "SysTime.h"

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
    bool isInitialized = isRangeSensorInitialized && isFlowSensorInitialized;

    if (isInitialized)
    {
        GetDeltaXYThread().detach();
        return true;
    }
    return false;
}

bool MotionSensor::IsConnected()
{
    return _rangeSensor.Initialize(false) && _opticalFLowSensor.Initialize();
}

bool MotionSensor::InitializeRangeSensor()
{
    const uint16_t timeout = 500;         // In milliseconds
    const uint32_t timingBudget = 200000; // In milliseconds, high accuracy mode

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
    const uint32_t timeBetweenReadings = 250; // In milliseconds

    while (true)
    {
        ReadRangeSensor();
        ReadFlowSensor();
        sleep_for_milliseconds(timeBetweenReadings);
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
    double travelInPixels = sqrt(static_cast<double>(((deltaX * deltaX) + (deltaY * deltaY))));
    uint16_t travelInMillimeter = PixelsToMillimeter(travelInPixels);
    _isMovingTravel += travelInMillimeter;
}

double MotionSensor::GetAverageRange()
{
    if (_rangeSensor.TimeoutOccurred())
    {
        printf("ERROR: Timeout occurred in range sensor \n");
    }
    return _rangeAverage.GetAverage();
}

bool MotionSensor::IsMoving()
{
    const uint16_t wheelchairMovingThreshold = 2500;

    if (_timer.Elapsed() >= WHEELCHAIR_MOVING_TIMEOUT.count() && _isMovingTravel < wheelchairMovingThreshold)
    {
        return false;
    }
    else if (_timer.Elapsed() < WHEELCHAIR_MOVING_TIMEOUT.count() && _isMovingTravel >= wheelchairMovingThreshold)
    {
        _isMovingTravel = 0;
        _deltaXAverage.Reset();
        _deltaYAverage.Reset();
        _rangeAverage.Reset();
        _timer.Reset();
        return true;
    }
    _timer.Reset();
    return false;
}

uint16_t MotionSensor::PixelsToMillimeter(double pixels)
{
    // The camera has a field of view of 42 degrees or 0.733038285 rad.
    // The sensor is 30 pixels by 30 pixels
    // We assume that the sensor is perpendicular to the ground.
    // Arc Length = fov_in_rad * height_from_the_ground
    const double numberOfPixels = 30.0f;
    const double fieldOfView = 0.733038285f;
    return static_cast<uint32_t>(((pixels * fieldOfView * GetAverageRange()) / numberOfPixels));
}
