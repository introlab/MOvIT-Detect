#ifndef ANGLE_FSM_H
#define ANGLE_FSM_H

#include "DataType.h"
#include <stdlib.h>
#include <stdio.h>

enum class AngleState
{
    INIT = 0,
    CONFIRM_ANGLE,
    ANGLE_STARTED,
    IN_TILT,
    CONFIRM_STOP_ANGLE,
    ANGLE_STOPPED
};

class AngleFSM
{
  public:
    void updateState(ChairState cs);
    long getStartTime();
    long getStopTime();
    int getCurrentState();
    long getElapsedTime() {
        return angleStopped - angleStarted;
    }
    char* getCurrentStateName() {
        return AngleStateName[getCurrentState()];
    }

    void setParameter(int frequency, int duration, int angle) {
        ANGLE_TARGET = angle;
        ANGLE_DURATION = duration;
        ANGLE_FREQUENCY = frequency;
    }

    int getTargetAngle() {
        return ANGLE_TARGET;
    }

    int getTargetDuration() {
        return ANGLE_DURATION;
    }

    int getTargetFrequency() {
        return ANGLE_FREQUENCY;
    }

    long getCurrentTime() {
        return currentTime;
    }

    void getTimePerAngle(int data[]) {
        data[0] = result[0];
        data[1] = result[1];
        data[2] = result[2];
        data[3] = result[3];
        data[4] = result[4];
    }

    int getAngleAverage() {
        return sum/dataPoints;
    }

    AngleFSM(int threshold = 12, int target = 30, int timeout = 2, int duration = 10);

    private:
        int ANGLE_TIMEOUT = 2;
        int ANGLE_THRESHOLD = 12;
        int REVERSE_ANGLE_THRESHOLD = -5;
        int ANGLE_TARGET = 30;
        int ANGLE_DURATION = 10;
        int ANGLE_FREQUENCY = 10;
        AngleState currentState = AngleState::INIT;
        int result[5] = {0,0,0,0,0};
        long lastTime = 0;
        int sum = 0;
        int dataPoints = 0;
        long angleStarted = 0;
        long angleStopped = 0;
        long currentTime = 0;
        char AngleStateName[6][22]  = {"INIT", "CONFIRM_ANGLE", "ANGLE_STARTED", "IN_TILT", "CONFIRM_STOP_ANGLE", "ANGLE_STOPPED"};
};

#endif //ANGLE_FSM_H