#ifndef TRAVEL_FSM_H
#define TRAVEL_FSM_H

#include "DataType.h"
#include <stdlib.h>
#include <stdio.h>

enum class TravelState {
    INIT = 0,
    WAITING_MOVEMENT,               //Confirmation du deplacement (timeout)
    TRAVEL_STARTED,                 //Le client a commence a ce deplacer pour vrai     LOG Debut travel
    ON_THE_MOVE,                    //Le client est en deplacement                     LOG Deplacement
    MOVEMENT_NOT_DETECTED,          //Le client a arreter le mouvement
    TRAVEL_STOPPED                  //Le client a vraiment arrete de ce deplacer       LOG Fin Travel
};

class TravelFSM
{
  public:
    void updateState(ChairState cs);
    long getStartTime();
    long getStopTime();
    int getCurrentState();
    long getElapsedTime() {
        return travelStoppedTime - travelStartedTime;
    }
    char* getCurrentStateName() {
        return TravelStateName[getCurrentState()];
    }

    void travelStartTimeout(int timeout) {
        TRAVEL_START_TIMEOUT = timeout;
    }

    void travelStopTimeout(int timeout) {
        TRAVEL_STOP_TIMEOUT = timeout;
    }

    void travelThreshold(int threshold) {
        TRAVEL_THRESHOLD = threshold;
    }

    long getCurrentTime() {
        return currentTime;
    }
    

    TravelFSM(int startTimeout = 1, int stopTimeout = 2, int threshold = 100);

    private:
        int TRAVEL_START_TIMEOUT = 1;                           // Temps avant confirmation de mouvement en secondes
        int TRAVEL_STOP_TIMEOUT = 2;                            // Temps avant confirmation d'arret en secondes
        int TRAVEL_THRESHOLD = 100;                             // Distance minimale pour confirmer un mouvement en mm
        TravelState currentState = TravelState::INIT;
        char TravelStateName[6][22] =  {"INIT", "WAITING_MOVEMENT", "TRAVEL_STARTED", "ON_THE_MOVE", "MOVEMENT_NOT_DETECTED", "TRAVEL_STOPPED"};
        long travelStartedTime = 0;
        long travelStoppedTime = 0;
        long lastTime = 0;
        int travelSum = 0;
        long currentTime = 0;
};

#endif //TRAVEL_FSM_H