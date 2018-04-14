#include "MotionSensor.h"
#include <math.h>

#define TIMEOUT 500
#define TIMING_BUDGET 200000 //High accuracy mode
#define TIME_BETWEEN_READINGS 1000
#define FIELD_OF_VIEW 42

void MotionSensor::Initialize()
{
    if (InitializeRangeSensor() && InitializeOpticalFlowSensor())
    {
        GetDeltaXYThread().detach();
    }
}

bool MotionSensor::InitializeRangeSensor()
{
    printf("Initialization of the range sensor... ");
    if (_rangeSensor.Initialize(false))
    {
        printf("FAIL \n");
        return true;
    }
    _rangeSensor.SetTimeout(TIMEOUT);
    _rangeSensor.SetMeasurementTimingBudget(TIMING_BUDGET);

    printf("SUCCESS \n");
    return false;
}

bool MotionSensor::InitializeOpticalFlowSensor()
{
    printf("Initialization of the flow sensor... ");
    if (_opticalFLowSensor.Initialize())
    {
        printf("FAIL \n");
        return true;
    }
    
    printf("SUCCESS \n");
    return false;
}

void MotionSensor::UpdateTravel(int16_t deltaX, int16_t deltaY)
{
    //Todo: To be implemented.
    _lastTravel = sqrtf((deltaX * deltaX) + (deltaY * deltaY));
#ifdef DEBUG_PRINT
    printf("---------------- \n");
    printf("Delta X %i: \n", deltaX);
    printf("Delta Y %i: \n", deltaY);
    printf("Travel %f: \n", _lastTravel);
#endif
}

void MotionSensor::GetDeltaXY()
{
    while (1)
    {
        int16_t deltaX = 0;
        int16_t deltaY = 0;

        _opticalFLowSensor.ReadMotionCount(&deltaX, &deltaY);
        UpdateTravel(deltaX, deltaY);
        SleepForMilliSeconds(TIME_BETWEEN_READINGS);
    }
}

uint16_t MotionSensor::GetRange()
{
    if (_rangeSensor.TimeoutOccurred())
    {
        printf("Timeout occurred in range sensor \n");
    }
    printf("Range: %i", _rangeSensor.ReadRangeSingleMillimeters());
    return _rangeSensor.ReadRangeSingleMillimeters();
}

std::thread MotionSensor::GetDeltaXYThread()
{
    return std::thread([=] { GetDeltaXY(); });
}

bool MotionSensor::GetIsMoving()
{
    //Todo: To be implemented.
    return false;
}

float MotionSensor::GetLastTravel()
{
    //Todo: To be implemented.
    return 0.0f;
}
