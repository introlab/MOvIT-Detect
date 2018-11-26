#include "FixedImu.h"
#include "Utils.h"
#include <string>
#include <unistd.h>

#define GRAVITY 9.80665

FixedImu::FixedImu()
{
    _imuName = FIXED_IMU_NAME;
    _imu = {0x68};
}

double FixedImu::GetXAcceleration()
{
    int16_t ax, ay, az;

    _imu.GetAcceleration(&ax, &ay, &az);

    const double g = -1;
    double accelerationGravity = static_cast<double>(ax) * 2 / 32768.0f;
    double accelerationMeterSquare = (accelerationGravity - g) * GRAVITY;

    return accelerationMeterSquare;
}
