#include "MotionSensor.h"
#include <math.h>
#include "Utils.h"

#define TIME_BETWEEN_READINGS 100   // In milliseconde
#define MINIMUM_WORKING_RANGE 0.08f //The sensor needs a minimum of 80 mm to the ground.

#define WHEELCHAIR_MOVING_THRESHOLD 0.25f // In meter
#define WHEELCHAIR_MOVING_TIMEOUT 5000    // In milliseconde

const char *FAIL_MESSAGE = "FAIL \n";
const char *SUCCESS_MESSAGE = "SUCCESS \n";

void MotionSensor::Initialize()
{
    if (InitializeRangeSensor() && InitializeOpticalFlowSensor() && ValidDistanceToTheGround())
    {
        GetDeltaXYThread().detach();
    }
}

bool MotionSensor::InitializeRangeSensor()
{
    uint16_t const timeout = 500;         // In milliseconde
    uint32_t const timingBudget = 200000; // In milliseconde, high accuracy mode

    printf("Initialization of the range sensor... ");
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
    printf("Initialization of the flow sensor... ");

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
    int16_t deltaX = 0;
    int16_t deltaY = 0;

    while (1)
    {
        _opticalFLowSensor.ReadMotionCount(&deltaX, &deltaY);
        UpdateTravel(&deltaX, &deltaY);
        sleep_for_milliseconds(TIME_BETWEEN_READINGS);
    }
}

void MotionSensor::UpdateTravel(int16_t *deltaX, int16_t *deltaY)
{
    float travelInPixels = sqrtf(float((*deltaX * *deltaX) + (*deltaY * *deltaY)));
    float travelInMeters = PixelsToMeters(travelInPixels);
    _lastTravel += travelInMeters;
    _isMovingTravel += travelInMeters;

#ifdef DEBUG_PRINT
    printf("Delta X %i: \n", *deltaX);
    printf("Delta Y %i: \n", *deltaY);
    printf("Travel %lf: \n", travelInMeters);
    printf("Is Moving travel %lf: \n", _isMovingTravel);

    std::ofstream file;

    file.open("MotionSensor.txt", std::ofstream::out | std::ofstream::app);

    if (!file.is_open())
    {
        return;
    }
    file << *deltaX << ";" << *deltaY << ";" << GetRangeInMeters() << ";" << travelInMeters << std::endl;
    file.close();
#endif
}

float MotionSensor::GetRangeInMeters()
{
    if (_rangeSensor.TimeoutOccurred())
    {
        printf("Timeout occurred in range sensor \n");
    }
    return float(_rangeSensor.ReadRangeSingleMillimeters()) / 1000.0f;
}

bool MotionSensor::ValidDistanceToTheGround()
{
    if (GetRangeInMeters() < MINIMUM_WORKING_RANGE)
    {
        printf("The flow sensor is too close to the ground. \n");
        return false;
    }
    return true;
}

bool MotionSensor::GetIsMoving()
{
    if (_timer.Elapsed() > WHEELCHAIR_MOVING_TIMEOUT && _isMovingTravel < WHEELCHAIR_MOVING_THRESHOLD)
    {
        return false;
    }
    if (_isMovingTravel >= WHEELCHAIR_MOVING_THRESHOLD)
    {
        _isMovingTravel = 0.0f;
        _timer.Reset();
    }
    return true;
}

float MotionSensor::GetLastTravel()
{
    // Add a mutex to avoid updating the value and read it at the same time
    float lastTravelBackup = _lastTravel;
    _lastTravel = 0.0f;
    return lastTravelBackup;
}

float MotionSensor::PixelsToMeters(float pixels)
{
    // The camera has a field of view of 42 degrees or 0.733038285 rad.
    // The sensor is 30 pixels by 30 pixels
    // We assume that the sensor is perpendicular to the ground.
    // Arc Length = fov_in_rad * height_from_the_ground
    float const nbOfPixels = 30.0f;
    float const fov = 0.733038285f;
    return (pixels * fov * GetRangeInMeters()) / nbOfPixels;
}
