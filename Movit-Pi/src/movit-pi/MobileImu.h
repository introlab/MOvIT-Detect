#ifndef MOBILE_IMU_H
#define MOBILE_IMU_H

#include "MPU6050.h"
#include "Imu.h"

class MobileImu : public Imu
{
  public:
    static MobileImu *GetInstance()
    {
      static MobileImu instance;
      return &instance;
    }

  private:
    MobileImu();
    MobileImu(MobileImu const &);      // Don't Implement.
    void operator=(MobileImu const &); // Don't implement.
};

#endif // MOBILE_IMU_H
