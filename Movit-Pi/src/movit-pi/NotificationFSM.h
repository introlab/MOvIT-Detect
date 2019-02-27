#ifndef NOTIFICATION_FSM_H
#define NOTIFICATION_FSM_H

#include "DataType.h"
#include <stdlib.h>
#include <stdio.h>
#include "AngleFSM.h"
#include "SeatingFSM.h"
#include "TravelFSM.h"

enum class NotificationState
{
    INIT = 0,
    WAIT_PERIOD,
    IN_TRAVEL,
    WAITING_FOR_TILT,
    TILT_SNOOZED,
    NOTIFICATION_TILT_STARTED,
    IN_TILT,
    TILT_DURATION_OK,
    NOTIFICATION_TILT_STOPPED
};

class NotificationFSM
{
  public:
    void updateState(ChairState cs, AngleFSM aFSM, SeatingFSM sFSM, TravelFSM tFSM);
    long getStartTime();
    long getStopTime();
    int getCurrentState();

    char* getCurrentStateName() {
        return notificationStateName[getCurrentState()];
    }

    NotificationFSM(int period = 5, int duration = 5, int snoozeTime = 5);

    private:
        int SNOOZE_TIME = 0;
        int PERIOD = 0;
        int DURATION = 0;
        NotificationState currentState = NotificationState::INIT;
        int MAX_SNOOZE = 2;
        int snoozeCount = 0;
        int startTime = 0;
        int stopTime = 0;
        int secondsCounter = 0;
        long currentTime = 0;
        char notificationStateName[9][26]  = {"INIT", "WAIT_PERIOD", "IN_TRAVEL", "WAITING_FOR_TILT", "TILT_SNOOZED", "NOTIFICATION_TILT_STARTED", "IN_TILT", "TILT_DURATION_OK", "NOTIFICATION_TILT_STOPPED"};
};

#endif //NOTIFICATION_FSM_H