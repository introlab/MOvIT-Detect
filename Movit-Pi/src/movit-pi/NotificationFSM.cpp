
#include "NotificationFSM.h"

void NotificationFSM::updateState(ChairState cs, AngleFSM aFSM, SeatingFSM sFSM, TravelFSM tFSM, bool isEnable)
{
    switch (currentState) {
        case NotificationState::INIT:
            stopReason = "Other";
            snoozeCount = 0;
            startTime = 0;
            secondsCounter = 0;
            stopTime = 0;
            if(sFSM.getCurrentState() == static_cast<int>(SeatingState::CURRENTLY_SEATING) ) {
                startTime = cs.time;
                currentState = NotificationState::WAIT_PERIOD;
            }

            if(!isEnable) {
                currentState = NotificationState::INIT;
                stopReason = "Other";
            }

        break;

        //##################################################

        case NotificationState::IN_TRAVEL_ELAPSED:
            if (tFSM.getCurrentState() == static_cast<int>(TravelState::ON_THE_MOVE)) {
                currentState = NotificationState::IN_TRAVEL_ELAPSED;
            } else {
                currentState = NotificationState::WAITING_FOR_TILT;
            }
        break;

        //##################################################

        case NotificationState::WAIT_PERIOD:
            stopReason = "Other";
            //La personne est en mouvement
            if (tFSM.getCurrentState() == static_cast<int>(TravelState::ON_THE_MOVE)) {
                currentState = NotificationState::IN_TRAVEL;
            }

            //La personne est assise
            if(sFSM.getCurrentState() == static_cast<int>(SeatingState::CURRENTLY_SEATING) ||
                    sFSM.getCurrentState() == static_cast<int>(SeatingState::CONFIRM_STOP_SEATING)) {
                if(currentTime != cs.time) {
                    secondsCounter++;
                    //printf ....*******************************************************************


                    //Le temps d'attente pour une bascule est ecoule
                    //DL Ou nous sommes deja en bascule
                    if(secondsCounter >= aFSM.getTargetFrequency() || aFSM.getCurrentState() == static_cast<int>(AngleState::IN_TILT)) {
                        currentState = NotificationState::NOTIFICATION_TILT_STARTED;
                    }                                                                                                    
                }
            } else {
                currentState = NotificationState::INIT;
            }

            if(!isEnable) {
                currentState = NotificationState::INIT;
                stopReason = "USER_DISABLED";
            }
        break;

        //##################################################

        case NotificationState::IN_TRAVEL:
            stopReason = "Other";
            if (tFSM.getCurrentState() == static_cast<int>(TravelState::ON_THE_MOVE)) {
                currentState = NotificationState::IN_TRAVEL;
            } else {
                currentState = NotificationState::WAIT_PERIOD;
            }
            //Même en déplacement on est assis...
            if(currentTime != cs.time) {
                secondsCounter++;
            }

            //La personne n'est plus assise
            if(!(sFSM.getCurrentState() == static_cast<int>(SeatingState::CURRENTLY_SEATING) || sFSM.getCurrentState() == static_cast<int>(SeatingState::CONFIRM_STOP_SEATING))) {
                currentState = NotificationState::INIT;
            }

            if(!isEnable) {
                currentState = NotificationState::INIT;
                stopReason = "USER_DISABLED";
            }
        break;

        //##################################################

        case NotificationState::WAITING_FOR_TILT:
            stopReason = "Other";
            secondsCounter = 0;
            if(cs.button) {
                snoozeCount++;
                if(snoozeCount >= 4) {
                    currentState = NotificationState::TILT_SNOOZED;
                    stopReason = "SNOOZED_REQUESTED";
                    secondsCounter = 0;
                }
            } else {
                snoozeCount = 0;
            }

            if (tFSM.getCurrentState() == static_cast<int>(TravelState::ON_THE_MOVE)) {
                currentState = NotificationState::IN_TRAVEL_ELAPSED;
                break;
            }

            //La personne n'est plus assise
            if(!(sFSM.getCurrentState() == static_cast<int>(SeatingState::CURRENTLY_SEATING) || sFSM.getCurrentState() == static_cast<int>(SeatingState::CONFIRM_STOP_SEATING))) {
                currentState = NotificationState::NOTIFICATION_TILT_STOPPED;
                stopReason = "NOT_SEATED";
                break;
            }

            //Une bascule est détecté ou la personne était déja en bascule
            if((aFSM.getCurrentState() == static_cast<int>(AngleState::ANGLE_STARTED)) || (aFSM.getCurrentState() == static_cast<int>(AngleState::IN_TILT))) {
                currentState = NotificationState::IN_TILT;
                stopReason = "TILT_BEGIN";
                secondsCounter = 0;
            }

            if(!isEnable) {
                currentState = NotificationState::INIT;
                stopReason = "USER_DISABLED";
            }

        break;

        //##################################################

        case NotificationState::TILT_SNOOZED:
            stopReason = "Other";
            if(currentTime != cs.time) {
                secondsCounter++;
                //Atteint le snooze time
                //DL ou la personne est deja en tilt
                if(secondsCounter >= SNOOZE_TIME || aFSM.getCurrentState() == static_cast<int>(AngleState::IN_TILT)) {
                    currentState = NotificationState::WAITING_FOR_TILT;
                    stopReason = "TILT_REQUESTED";
                }
            }

            //La personne n'est plus assise
            if(!(sFSM.getCurrentState() == static_cast<int>(SeatingState::CURRENTLY_SEATING) || sFSM.getCurrentState() == static_cast<int>(SeatingState::CONFIRM_STOP_SEATING))) {
                currentState = NotificationState::NOTIFICATION_TILT_STOPPED;
                stopReason = "NOT_SEATED";
            }

            if(!isEnable) {
                currentState = NotificationState::INIT;
                stopReason = "USER_DISABLED";
            }

        break;

        //##################################################

        case NotificationState::NOTIFICATION_TILT_STARTED:
            stopReason = "Other";
            currentState = NotificationState::WAITING_FOR_TILT;
            stopReason = "INITIAL_TILT_REQUESTED";
        break;

        //##################################################

        case NotificationState::IN_TILT:
            stopReason = "Other";
            //La personne n'est plus assise
            if(!(sFSM.getCurrentState() == static_cast<int>(SeatingState::CURRENTLY_SEATING) || sFSM.getCurrentState() == static_cast<int>(SeatingState::CONFIRM_STOP_SEATING))) {
                currentState = NotificationState::NOTIFICATION_TILT_STOPPED;
                stopReason = "NOT_SEATED";
                break;
            }

            if(currentTime != cs.time) {
                secondsCounter++;
            }

            if((aFSM.getCurrentState() == static_cast<int>(AngleState::IN_TILT)) || (aFSM.getCurrentState() == static_cast<int>(AngleState::CONFIRM_STOP_ANGLE))) {
                if(aFSM.getTargetDuration() <= secondsCounter) {
                    currentState = NotificationState::TILT_DURATION_OK;
                } else {
                    currentState = NotificationState::IN_TILT;
                }
                //La bascule est terminée
            } else if (aFSM.getCurrentState() == static_cast<int>(AngleState::ANGLE_STOPPED)) {
                currentState = NotificationState::NOTIFICATION_TILT_STOPPED;
                stopReason = "END_OF_TILT";
            }

            if(!isEnable) {
                currentState = NotificationState::NOTIFICATION_TILT_STOPPED;
                stopReason = "USER_DISABLED";
            }
        
        break;

        //##################################################

        case NotificationState::TILT_DURATION_OK:
            //La personne n'est plus assise
            if(!(sFSM.getCurrentState() == static_cast<int>(SeatingState::CURRENTLY_SEATING) || sFSM.getCurrentState() == static_cast<int>(SeatingState::CONFIRM_STOP_SEATING))) {
                currentState = NotificationState::NOTIFICATION_TILT_STOPPED;
                stopReason = "NOT_SEATED";
                break;
            }

            if(currentTime != cs.time) {
                secondsCounter++;
            }

            if((aFSM.getCurrentState() == static_cast<int>(AngleState::IN_TILT)) || (aFSM.getCurrentState() == static_cast<int>(AngleState::CONFIRM_STOP_ANGLE))) {
                currentState = NotificationState::TILT_DURATION_OK;
                //La bascule est terminée
            } else if (aFSM.getCurrentState() == static_cast<int>(AngleState::ANGLE_STOPPED)) {
                currentState = NotificationState::NOTIFICATION_TILT_STOPPED;
                stopReason = "END_OF_TILT_OK";
            }

            if(!isEnable) {
                currentState = NotificationState::NOTIFICATION_TILT_STOPPED;
                stopReason = "USER_DISABLED";
            }

        break;

        //##################################################
        
        case NotificationState::NOTIFICATION_TILT_STOPPED:
            stopReason = "NOTIFICATION_COMPLETE";
            currentState = NotificationState::INIT;
        break;
    }
    currentTime = cs.time;
}

int NotificationFSM::getCurrentState()
{
    return static_cast<int>(currentState);
}

NotificationFSM::NotificationFSM(int snoozeTime) {
    SNOOZE_TIME = snoozeTime;
}
