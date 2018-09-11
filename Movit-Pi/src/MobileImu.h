#ifndef MOBILE_IMU_H
#define MOBILE_IMU_H

#include "MPU6050.h"
#include "Imu.h"

class MobileImu : public Imu
{
  public:
    bool isSetup();
    static MobileImu *GetInstance()
    {
        static MobileImu instance;
        return &instance;
    }

  private:
    MobileImu();
};

#endif // MOBILE_IMU_H
