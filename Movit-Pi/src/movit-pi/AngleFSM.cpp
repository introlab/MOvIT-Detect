
#include "AngleFSM.h"

void AngleFSM::updateState(ChairState cs)
{
    switch (currentState) {
        case AngleState::INIT:
            if (cs.seatAngle >= ANGLE_THRESHOLD || cs.seatAngle <= REVERSE_ANGLE_THRESHOLD)
            {
                currentState = AngleState::CONFIRM_ANGLE;
                angleStarted = cs.time;
                angleStopped = cs.time;
                result[0] = 0;
                result[1] = 0;
                result[2] = 0;
                result[3] = 0;
                result[4] = 0;
                sum = 0;
                dataPoints = 0;
            } else {
                angleStarted = 0;
                angleStopped = 0;
            }
        break;
        
        case AngleState::CONFIRM_ANGLE:
            if (cs.seatAngle < ANGLE_THRESHOLD && cs.seatAngle > REVERSE_ANGLE_THRESHOLD)
            {
                currentState = AngleState::INIT;
                angleStarted = 0;
            } else if((cs.time - angleStarted) > ANGLE_TIMEOUT) {
                angleStarted = cs.time;
                currentState = AngleState::ANGLE_STARTED;
            }
            angleStopped = cs.time;
        break;
        
        case AngleState::ANGLE_STARTED:
            angleStarted = cs.time;
            currentState = AngleState::IN_TILT;
        break;
        
        case AngleState::IN_TILT:
            if ((cs.time - lastTime) >= 1)
            {
                if (cs.seatAngle < 0)
                {
                    result[0]++;
                }
                else if (cs.seatAngle >= 0 && cs.seatAngle < 15)
                {
                    result[1]++;
                }
                else if (cs.seatAngle >= 15 && cs.seatAngle < 30)
                {
                    result[2]++;
                }
                else if (cs.seatAngle >= 30 && cs.seatAngle < 45)
                {
                    result[3]++;
                }
                else
                {
                    result[4]++;
                }
                lastTime = cs.time;
            }

            dataPoints++;
            sum += cs.seatAngle;

            if(cs.seatAngle < ANGLE_THRESHOLD && cs.seatAngle > REVERSE_ANGLE_THRESHOLD) {
                angleStopped = cs.time;
                currentState = AngleState::CONFIRM_STOP_ANGLE;
            }
            angleStopped = cs.time;
        break;
       
        case AngleState::CONFIRM_STOP_ANGLE:
            if(cs.seatAngle >= ANGLE_THRESHOLD || cs.seatAngle <= REVERSE_ANGLE_THRESHOLD) {
                currentState = AngleState::IN_TILT;
                angleStopped = 0;
            } else {
                if((cs.time - angleStopped) > ANGLE_TIMEOUT) {
                    currentState = AngleState::ANGLE_STOPPED;
                }
            }
        break;
        
        case AngleState::ANGLE_STOPPED:
            currentState = AngleState::INIT;
        break;
    }
    currentTime = cs.time;
}

long AngleFSM::getStartTime() {
    return angleStarted;
}

long AngleFSM::getStopTime() {
    return angleStopped;
}

int AngleFSM::getCurrentState()
{
    return static_cast<int>(currentState);
}

AngleFSM::AngleFSM(int threshold, int target, int timeout, int duration) {
    ANGLE_THRESHOLD = threshold;
    ANGLE_TARGET = target;
    ANGLE_TIMEOUT = timeout;
    ANGLE_DURATION = duration;
}