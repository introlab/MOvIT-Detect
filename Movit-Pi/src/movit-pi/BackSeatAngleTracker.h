#ifndef BACK_SEAT_ANGLE_TRACKER_H
#define BACK_SEAT_ANGLE_TRACKER_H

#include "MovingAverage.h"

#define NUMBER_OF_AXIS 3
#define ALLOWED_INCLINATION_ANGLE 10

class BackSeatAngleTracker
{
  public:
    BackSeatAngleTracker();
    bool IsInclined();
    int GetBackSeatAngle();

  private:
    MovingAverage<int> _angle;
};

#endif // BACK_SEAT_ANGLE_TRACKER_H
