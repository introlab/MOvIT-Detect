#ifndef FIXED_IMU_H
#define FIXED_IMU_H

#include "MPU6050.h"
#include "Imu.h"

class FixedImu : public Imu
{
  public:
    double GetXAcceleration();
    static FixedImu *GetInstance()
    {
        static FixedImu instance;
        return &instance;
    }

  private:
    //Singleton
    FixedImu();
    FixedImu(FixedImu const &);       // Don't Implement.
    void operator=(FixedImu const &); // Don't implement.
};

#endif // FIXED_H
