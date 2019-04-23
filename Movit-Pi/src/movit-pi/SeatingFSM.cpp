
#include "SeatingFSM.h"

void SeatingFSM::updateState(ChairState cs)
{
    switch (currentState)
    {
        case SeatingState::INIT:
            if (cs.isSeated)
            {
                currentState = SeatingState::CONFIRM_SEATING;
                seatingStarted = cs.time;
                seatingStopped = cs.time;
            } else {
                seatingStarted = 0;
                seatingStopped = 0;
            }
            break;

        case SeatingState::CONFIRM_SEATING:
            if (!cs.isSeated)
            {
                currentState = SeatingState::INIT;
                seatingStarted = 0;
            } else if((cs.time - seatingStarted) > SEATING_TIMEOUT) {
                seatingStarted = cs.time;
                currentState = SeatingState::SEATING_STARTED;
            }
            seatingStopped = cs.time;
        break;

        case SeatingState::SEATING_STARTED:
            currentState = SeatingState::CURRENTLY_SEATING;
        break;

        case SeatingState::CURRENTLY_SEATING:
            if(!cs.isSeated) {
                currentState = SeatingState::CONFIRM_STOP_SEATING;
            }
            seatingStopped = cs.time;
        break;

        case SeatingState::CONFIRM_STOP_SEATING:
            if(cs.isSeated) {
                currentState = SeatingState::CURRENTLY_SEATING;
                seatingStopped = 0;
            } else {
                if((cs.time - seatingStopped) > SEATING_TIMEOUT) {
                    currentState = SeatingState::SEATING_STOPPED;
                }
            }
        break;
        
        case SeatingState::SEATING_STOPPED:
            currentState = SeatingState::INIT;
        break;
    }
    currentTime = cs.time;
}

long SeatingFSM::getStartTime() {
    return seatingStarted;
}

long SeatingFSM::getStopTime() {
    return seatingStopped;
}

int SeatingFSM::getCurrentState()
{
    return static_cast<int>(currentState);
}

SeatingFSM::SeatingFSM(int timeout)
{
    SEATING_TIMEOUT = timeout;
}