#ifndef MOVING_SENSOR_H
#define MOVING_SENSOR_H

#include <iostream>
#include <fstream>
#include <stdio.h>
#include <thread>

#include "PMW3901.h"
#include "VL53L0X.h"
#include "Utils.h"
#include "Timer.h"

class MotionSensor
{
  public:
    void Initialize();
    float GetLastTravel();
    bool GetIsMoving();

  private:
    std::thread GetDeltaXYThread();
    bool InitializeOpticalFlowSensor();
    bool InitializeRangeSensor();
    bool ValidDistanceToTheGround();

    float GetRangeInMeters();
    void GetDeltaXY();

    void UpdateTravel(int16_t *deltaX, int16_t *deltaY);
    float PixelsToMeters(float pixels);

    float _lastTravel = 0.0f;
    float _isMovingTravel = 0.0f;

    std::chrono::high_resolution_clock::time_point _timeoutStartMs;
    PMW3901 _opticalFLowSensor; // Optical Flow Sensor
    VL53L0X _rangeSensor;       // Range Sensor

    Timer _timer;
};

#endif // MOVING_SENSOR_H
