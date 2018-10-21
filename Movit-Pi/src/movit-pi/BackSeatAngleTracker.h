#ifndef BACK_SEAT_ANGLE_TRACKER_H
#define BACK_SEAT_ANGLE_TRACKER_H

#define NUMBER_OF_AXIS 3
#define ALLOWED_INCLINATION_ANGLE 10

class BackSeatAngleTracker
{
  public:
    BackSeatAngleTracker();
    bool IsInclined();
    int GetBackSeatAngle();
};

#endif // BACK_SEAT_ANGLE_TRACKER_H
