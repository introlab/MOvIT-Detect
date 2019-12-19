
#include "TravelFSM.h"

void TravelFSM::updateState(ChairState cs)
{
    switch (currentState)
    {
        case TravelState::INIT:

            if(cs.lastDistance > TRAVEL_THRESHOLD) {
                //La distance minimale est parcouru
                lastTime = cs.time;
                currentState = TravelState::WAITING_MOVEMENT;
            } else {
                //La distance minimale n'est pas parcouru
                travelStartedTime = 0;
                travelStoppedTime = 0;
            }

            break;
        
        case TravelState::WAITING_MOVEMENT:
            if((cs.time - lastTime) > 1) {
                //Deux secondes sont écoulé
                if(travelSum > TRAVEL_THRESHOLD) {
                    //La distance minimale est parcouru
                    currentState = TravelState::TRAVEL_STARTED;
                    travelSum = 0;
                } else {
                    //La distance minimale n'est pas parcouru
                    currentState = TravelState::INIT;
                    travelSum = 0;
                }
                lastTime = cs.time;
                
            } else {
                //Pas passé le delai, on integre encore
                travelSum += cs.lastDistance;
            }
            break;
        
        case TravelState::TRAVEL_STARTED:
            currentState = TravelState::ON_THE_MOVE;
            travelStartedTime = cs.time;
            travelStoppedTime = cs.time;
            //printf("Travel started at: %ld\n", travelStartedTime);
            break;
        
        case TravelState::ON_THE_MOVE:
            if(cs.lastDistance < 5) {
                lastTime = cs.time;
                travelStoppedTime = cs.time;
                currentState = TravelState::MOVEMENT_NOT_DETECTED;
            } else {
                //printf("Last distance: %d\n", cs.lastDistance);
                travelStoppedTime = cs.time;
            }
            break;
        
        case TravelState::MOVEMENT_NOT_DETECTED:
            if((cs.time-lastTime) > TRAVEL_STOP_TIMEOUT) {
                if(travelSum > 25) {
                    //La distance minimale est parcouru
                    currentState = TravelState::ON_THE_MOVE;
                    travelSum = 0;
                } else {
                    //La distance minimale n'est pas parcouru
                    currentState = TravelState::TRAVEL_STOPPED;
                    travelSum = 0;
                }
                lastTime = cs.time;
            } else {
                //Pas passé le delai, on integre encore
                travelSum += cs.lastDistance;
                travelStoppedTime = cs.time;
            }
            break;
        
        case TravelState::TRAVEL_STOPPED:
            currentState = TravelState::INIT;
            //printf("Travel stopped at: %ld\n", travelStoppedTime);
            //printf("Moved for: %ld\n", travelStoppedTime-travelStartedTime);
            break;
    }
    currentTime = cs.time;
}

long TravelFSM::getStartTime() {
    return travelStartedTime;
}

long TravelFSM::getStopTime() {
    return travelStoppedTime;
}

int TravelFSM::getCurrentState()
{
    return static_cast<int>(currentState);
}

TravelFSM:: TravelFSM(int startTimeout, int stopTimeout, int threshold)
{
    TRAVEL_START_TIMEOUT = startTimeout;
    TRAVEL_STOP_TIMEOUT = stopTimeout;
    TRAVEL_THRESHOLD = threshold;
}