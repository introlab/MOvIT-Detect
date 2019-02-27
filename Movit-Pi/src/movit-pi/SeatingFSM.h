#ifndef SEATING_FSM_H
#define SEATING_FSM_H

#include "DataType.h"
#include <stdlib.h>
#include <stdio.h>

enum class SeatingState
{
    INIT = 0,
    CONFIRM_SEATING,
    SEATING_STARTED,
    CURRENTLY_SEATING,
    CONFIRM_STOP_SEATING,
    SEATING_STOPPED
};

class SeatingFSM
{
  public:
    void updateState(ChairState cs);
    long getStartTime();
    long getStopTime();
    int getCurrentState();
    long getElapsedTime() {
        return seatingStopped - seatingStarted;
    }
    char* getCurrentStateName() {
        return SeatingStateName[getCurrentState()];
    }

    void setSeatingTimeout(int timeout) {
        SEATING_TIMEOUT = timeout;
    }

    long getCurrentTime() {
        return currentTime;
    }
    

    SeatingFSM(int timeout = 5);

    private:
        int SEATING_TIMEOUT = 5;                          //En secondes
        SeatingState currentState = SeatingState::INIT;
        long seatingStarted = 0;
        long seatingStopped = 0;
        char SeatingStateName[6][22]  = {"INIT", "CONFIRM_SEATING", "SEATING_STARTED", "CURRENTLY_SEATING", "CONFIRM_STOP_SEATING", "SEATING_STOPPED"};
        long currentTime = 0;
};

#endif //SEATING_FSM_H