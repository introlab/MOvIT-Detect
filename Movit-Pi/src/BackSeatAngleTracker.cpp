#include "BackSeatAngleTracker.h"
#include "FixedImu.h"
#include "MobileImu.h"
#include "Utils.h"
#include <math.h>

BackSeatAngleTracker::BackSeatAngleTracker()
{
}

bool BackSeatAngleTracker::IsInclined()
{
    FixedImu *fixedImu = FixedImu::GetInstance();
    double pitch = fixedImu->GetPitch();
    double roll = fixedImu->GetRoll();

    return pitch > ALLOWED_INCLINATION_ANGLE || pitch < ALLOWED_INCLINATION_ANGLE * -1
        || roll > ALLOWED_INCLINATION_ANGLE || roll < ALLOWED_INCLINATION_ANGLE * -1;
}

int BackSeatAngleTracker::GetBackSeatAngle()
{
    MobileImu *mobileImu = MobileImu::GetInstance();
    FixedImu *fixedImu = FixedImu::GetInstance();

    double fixedPitch = mobileImu->GetPitch();
    double mobilePitch = fixedImu->GetPitch();

    return int(fixedPitch - mobilePitch);
}
