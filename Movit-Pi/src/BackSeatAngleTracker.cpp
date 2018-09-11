#include "BackSeatAngleTracker.h"
#include "FixedImu.h"
#include "MobileImu.h"
#include "Utils.h"
#include <math.h>

BackSeatAngleTracker::BackSeatAngleTracker()
{
}

double BackSeatAngleTracker::GetPitch(double acceleration[])
{
    if (acceleration[AXIS::z] > 0)
    {
        return atan2(acceleration[AXIS::x], -1 * sqrt(acceleration[AXIS::y] * acceleration[AXIS::y] + acceleration[AXIS::z] * acceleration[AXIS::z])) * radiansToDegrees;
    }
    else
    {
        return atan2(acceleration[AXIS::x], sqrt(acceleration[AXIS::y] * acceleration[AXIS::y] + acceleration[AXIS::z] * acceleration[AXIS::z])) * radiansToDegrees;
    }
}

int BackSeatAngleTracker::GetBackSeatAngle()
{
    double fixedImuAccelerations[NUMBER_OF_AXIS] = {0, 0, 0};
    double mobileImuAccelerations[NUMBER_OF_AXIS] = {0, 0, 0};
 
    MobileImu *mobileImu = MobileImu::GetInstance();
    FixedImu *fixedImu = FixedImu::GetInstance();
    fixedImu->GetAccelerations(fixedImuAccelerations);
    mobileImu->GetAccelerations(mobileImuAccelerations);

    double fixedPitch = GetPitch(fixedImuAccelerations);
    double mobilePitch = GetPitch(mobileImuAccelerations);

    return int(fixedPitch - mobilePitch);
}
