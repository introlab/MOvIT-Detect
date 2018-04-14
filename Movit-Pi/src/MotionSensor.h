#ifndef MOVING_SENSOR_H
#define MOVING_SENSOR_H

#include <stdio.h>
#include <thread>

#include "PMW3901/PMW3901.h"
#include "VL53L0X/VL53L0X.h"
#include "Utils.h"

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
    uint16_t GetRange();

    void UpdateTravel(int16_t deltaX, int16_t deltaY);
    void GetDeltaXY();

    float _lastTravel = 0.0f;

    PMW3901 _opticalFLowSensor; // Optical Flow Sensor
    VL53L0X _rangeSensor;       // Range Sensor
};

#endif // MOVING_SENSOR_H