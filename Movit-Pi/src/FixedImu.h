#ifndef FIXED_IMU_H
#define FIXED_IMU_H

#include "MPU6050.h"
#include "Imu.h"

class FixedImu : public Imu
{
  public:
    bool isSetup();
    double GetXAcceleration();
    static FixedImu *GetInstance()
    {
        static FixedImu instance;
        return &instance;
    }

  private:
    FixedImu();
};

#endif // FIXED_H
