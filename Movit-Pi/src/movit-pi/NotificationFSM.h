#ifndef NOTIFICATION_FSM_H
#define NOTIFICATION_FSM_H

#include "DataType.h"
#include <stdlib.h>
#include <stdio.h>
#include <string>
#include "AngleFSM.h"
#include "SeatingFSM.h"
#include "TravelFSM.h"

enum class NotificationState
{
    INIT = 0,                       //État d'initialisation (La personne pas assise)
    WAIT_PERIOD,                    //Vérification si le client est en mouvement ou si 
    IN_TRAVEL,                      //
    WAITING_FOR_TILT,               //
    TILT_SNOOZED,                   //Le client a appuyé sur le bouton
    NOTIFICATION_TILT_STARTED,      //La personne est assise ou la bascule est commencée
    IN_TILT,                        //Une bascule est détecté ou la personne était déja en bascule
    TILT_DURATION_OK,               //
    NOTIFICATION_TILT_STOPPED,      //La personne n'est plus assise ou la bascule est terminée
    IN_TRAVEL_ELAPSED               //
};

class NotificationFSM
{
  public:
    void updateState(ChairState cs, AngleFSM aFSM, SeatingFSM sFSM, TravelFSM tFSM, bool isEnable);
    long getStartTime();
    long getStopTime();
    int getCurrentState();
    long getCurrentTime() {
        return currentTime;
    }

    char* getCurrentStateName() {
        return notificationStateName[getCurrentState()];
    }

    NotificationFSM(int snoozeTime = 5);

    void setSnoozeTime(int time) {
        SNOOZE_TIME = time;
    }

    int getSnoozeTime() {
        return SNOOZE_TIME;
    }

    int getElapsed() {
        return secondsCounter;
    }

    std::string getReason() {
        return stopReason;
    }

    private:
        int SNOOZE_TIME = 0;
        std::string stopReason;
        NotificationState currentState = NotificationState::INIT;
        int MAX_SNOOZE = 2;
        int snoozeCount = 0;
        int startTime = 0;
        int stopTime = 0;
        int secondsCounter = 0;
        long currentTime = 0;
        char notificationStateName[10][26]  = {"INIT", "WAIT_PERIOD", "IN_TRAVEL", "WAITING_FOR_TILT", "TILT_SNOOZED", "NOTIFICATION_TILT_STARTED", "IN_TILT", "TILT_DURATION_OK", "NOTIFICATION_TILT_STOPPED", "IN_TRAVEL_ELAPSED"};
};

#endif //NOTIFICATION_FSM_H