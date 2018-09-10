#include "BackSeatAngleTracker.h"
#include "Utils.h"
#include <math.h>

enum _axis { x, y, z };
const std::string fixedImuName = "fixedImu";
const std::string mobileImuName = "mobileImu";

BackSeatAngleTracker::BackSeatAngleTracker()
{
}

double BackSeatAngleTracker::GetPitch(double acceleration[])
{
    return atan2(acceleration[_axis::x], sqrt(acceleration[_axis::y] * acceleration[_axis::y] + acceleration[_axis::z] * acceleration[_axis::z])) * radiansToDegrees;
}

int BackSeatAngleTracker::GetBackSeatAngle()
{
    double fixedImuAccelerations[NUMBER_OF_AXIS] = {0, 0, 0};
    double mobileImuAccelerations[NUMBER_OF_AXIS] = {0, 0, 0};

    _imu.GetAcceleration(fixedImuAccelerations, fixedImuName);
    _imu.GetAcceleration(mobileImuAccelerations, mobileImuName);

    double fixedPitch = GetPitch(fixedImuAccelerations);
    double mobilePitch = GetPitch(mobileImuAccelerations);

    return abs(int(fixedPitch - mobilePitch));
}
