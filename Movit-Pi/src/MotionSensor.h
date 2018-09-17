#ifndef MOVING_SENSOR_H
#define MOVING_SENSOR_H

#include <iostream>
#include <fstream>
#include <stdio.h>
#include <thread>

#include "MovingAverage.h"
#include "PMW3901.h"
#include "VL53L0X.h"
#include "Utils.h"
#include "Timer.h"

class MotionSensor
{
  public:
    // Singleton
    static MotionSensor *GetInstance()
    {
      static MotionSensor instance;
      return &instance;
    }

    void Initialize();
    bool GetIsMoving();

  private:
    //Singleton
    MotionSensor();
    MotionSensor(MotionSensor const &);   // Don't Implement.
    void operator=(MotionSensor const &); // Don't implement.

    std::thread GetDeltaXYThread();
    bool InitializeOpticalFlowSensor();
    bool InitializeRangeSensor();
    bool ValidDistanceToTheGround();

    uint16_t PixelsToMillimeter(double pixels);
    double GetAverageRange();

    void ReadRangeSensor();
    void ReadFlowSensor();
    void UpdateTravel();
    void GetDeltaXY();

    std::chrono::high_resolution_clock::time_point _timeoutStartMs;
    PMW3901 _opticalFLowSensor; // Optical Flow Sensor
    VL53L0X _rangeSensor;       // Range Sensor
    uint16_t _isMovingTravel;
    MovingAverage<uint16_t> *_rangeAverage;
    MovingAverage<int16_t> *_deltaXAverage;
    MovingAverage<int16_t> *_deltaYAverage;
    Timer _timer;
};

#endif // MOVING_SENSOR_H
