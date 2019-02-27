
#include "NotificationFSM.h"

void NotificationFSM::updateState(ChairState cs, AngleFSM aFSM, SeatingFSM sFSM, TravelFSM tFSM)
{
    switch (currentState) {
        case NotificationState::INIT:
            snoozeCount = 0;
            startTime = 0;
            secondsCounter = 0;
            stopTime = 0;
            if(sFSM.getCurrentState() == static_cast<int>(SeatingState::CURRENTLY_SEATING)) {
                startTime = cs.time;
                currentState = NotificationState::WAIT_PERIOD;
            }
        break;
        case NotificationState::WAIT_PERIOD:
            //La personne est en mouvement
            if (tFSM.getCurrentState() == static_cast<int>(TravelState::ON_THE_MOVE)) {
                currentState = NotificationState::IN_TRAVEL;
            }

            //La personne est assise
            if(sFSM.getCurrentState() == static_cast<int>(SeatingState::CURRENTLY_SEATING) || sFSM.getCurrentState() == static_cast<int>(SeatingState::CONFIRM_STOP_SEATING)) {
                if(currentTime != cs.time) {
                    secondsCounter++;
                    if(secondsCounter >= PERIOD) {
                        currentState = NotificationState::WAITING_FOR_TILT;
                    }
                }
            } else {
                currentState = NotificationState::INIT;
            }
        break;
        case NotificationState::IN_TRAVEL:
            if (tFSM.getCurrentState() == static_cast<int>(TravelState::ON_THE_MOVE)) {
                currentState = NotificationState::IN_TRAVEL;
            } else {
                currentState = NotificationState::WAIT_PERIOD;
            }
            //La personne N'est plus assise
            if(!(sFSM.getCurrentState() == static_cast<int>(SeatingState::CURRENTLY_SEATING) || sFSM.getCurrentState() == static_cast<int>(SeatingState::CONFIRM_STOP_SEATING))) {
                currentState = NotificationState::INIT;
            }
        break;
        case NotificationState::WAITING_FOR_TILT:
            if(cs.button) {
                currentState = NotificationState::TILT_SNOOZED;
                snoozeCount++;
                secondsCounter = 0;
            }
        
        break;
        case NotificationState::TILT_SNOOZED:
            if(currentTime != cs.time) {
                secondsCounter++;
                if(secondsCounter >= SNOOZE_TIME) {
                    currentState = NotificationState::WAITING_FOR_TILT;
                    secondsCounter = 0;
                }
            }
        break;
        case NotificationState::NOTIFICATION_TILT_STARTED:
        
        break;
        case NotificationState::IN_TILT:
        
        break;
        case NotificationState::TILT_DURATION_OK:
        
        break;
        case NotificationState::NOTIFICATION_TILT_STOPPED:
        
        break;
    }
    currentTime = cs.time;
}

int NotificationFSM::getCurrentState()
{
    return static_cast<int>(currentState);
}

NotificationFSM::NotificationFSM(int period, int duration, int snoozeTime) {
    SNOOZE_TIME = snoozeTime;
    PERIOD = period;
    DURATION = duration;
}