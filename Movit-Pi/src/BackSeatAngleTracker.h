#ifndef BACK_SEAT_ANGLE_TRACKER_H
#define BACK_SEAT_ANGLE_TRACKER_H

#define NUMBER_OF_AXIS 3

class BackSeatAngleTracker
{
  public:
    BackSeatAngleTracker();
    int GetBackSeatAngle();

  private:
    double GetPitch(double acceleration[]);
};

#endif // BACK_SEAT_ANGLE_TRACKER_H
