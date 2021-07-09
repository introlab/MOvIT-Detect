from datetime import datetime
import asyncio
import aiofiles
from contextlib import AsyncExitStack, asynccontextmanager
from asyncio_mqtt import Client, MqttError
from ChairState import ChairState, TravelInformation, AngleInformation, PressureInformation
from AngleFSM import AngleFSMState
from TravelFSM import TravelFSMState
from SeatingFSM import SeatingFSMState

import json
from enum import Enum, unique


class NotificationFSMState:
    NOTIFICATION_ENABLED = False
    LED_BLINK = False
    MOTOR_VIBRATION = False
    SNOOZE_TIME = 5
    MAX_SNOOZE = 2
    PUSH_SNOOZE_COUNT = 2

    @classmethod
    def setParameters(cls, enabled=False, ledBlink=False, motorVibration=False, snoozeTime=5):
        NotificationFSMState.NOTIFICATION_ENABLED = enabled
        NotificationFSMState.LED_BLINK = ledBlink
        NotificationFSMState.MOTOR_VIBRATION = motorVibration
        NotificationFSMState.SNOOZE_TIME = snoozeTime

    @classmethod
    def setSnoozeTime(cls, snooze_time: int):
        NotificationFSMState.SNOOZE_TIME = snooze_time

    @classmethod
    def setMaxSnooze(cls, max_snooze: int):
        NotificationFSMState.MAX_SNOOZE = max_snooze

    class NotificationState(Enum):
        INIT = 0
        WAIT_PERIOD = 1
        IN_TRAVEL = 2
        WAITING_FOR_TILT = 3
        TILT_SNOOZED = 4
        NOTIFICATION_TILT_STARTED = 5
        IN_TILT = 6
        TILT_DURATION_OK = 7
        NOTIFICATION_TILT_STOPPED = 8
        IN_TRAVEL_ELAPSED = 9

        @classmethod
        def from_name(cls, name: str):
            if name == NotificationFSMState.NotificationState.INIT.name:
                return NotificationFSMState.NotificationState.INIT
            elif name == NotificationFSMState.NotificationState.WAIT_PERIOD.name:
                return NotificationFSMState.NotificationState.WAIT_PERIOD
            elif name == NotificationFSMState.NotificationState.IN_TRAVEL.name:
                return NotificationFSMState.NotificationState.IN_TRAVEL
            elif name == NotificationFSMState.NotificationState.WAITING_FOR_TILT.name:
                return NotificationFSMState.NotificationState.WAITING_FOR_TILT
            elif name == NotificationFSMState.NotificationState.TILT_SNOOZED.name:
                return NotificationFSMState.NotificationState.TILT_SNOOZED
            elif name == NotificationFSMState.NotificationState.NOTIFICATION_TILT_STARTED.name:
                return NotificationFSMState.NotificationState.NOTIFICATION_TILT_STARTED
            elif name == NotificationFSMState.NotificationState.IN_TILT.name:
                return NotificationFSMState.NotificationState.IN_TILT
            elif name == NotificationFSMState.NotificationState.TILT_DURATION_OK.name:
                return NotificationFSMState.NotificationState.TILT_DURATION_OK
            elif name == NotificationFSMState.NotificationState.NOTIFICATION_TILT_STOPPED.name:
                return NotificationFSMState.NotificationState.NOTIFICATION_TILT_STOPPED
            elif name == NotificationFSMState.NotificationState.IN_TRAVEL_ELAPSED.name:
                return NotificationFSMState.NotificationState.IN_TRAVEL_ELAPSED
            else:
                raise ValueError('{} is not a valid NotificationState name'.format(name))

    def __init__(self,config):
        self.__type = 'NotificationFSMState'
        self.__currentState = NotificationFSMState.NotificationState.INIT
        self.__snoozeCount = 0
        self.__startTime = 0
        self.__stopTime = 0
        self.__secondsCounter = 0
        self.__currentTime = 0
        self.__stopReason = ''
        self.__snoozeRepetition = 0
        self.__secondsWaitingForTilt = 0

        if config.has_section('NotificationFSM'):
            NotificationFSMState.MAX_SNOOZE = config.getint('NotificationFSM','MAX_SNOOZE')
            NotificationFSMState.PUSH_SNOOZE_COUNT = config.getfloat('NotificationFSM','PUSH_SNOOZE_COUNT')


    def reset(self):
        self.__currentState = NotificationFSMState.NotificationState.INIT
        self.__snoozeCount = 0
        self.__startTime = 0
        self.__stopTime = 0
        self.__secondsCounter = 0
        self.__currentTime = 0
        self.__stopReason = ''
        self.__snoozeRepetition = 0
        self.__secondsWaitingForTilt = 0

    def in_state(self, state: NotificationState):
        return self.__currentState == state

    def getStartTime(self):
        return self.__startTime

    def getStopTime(self):
        return self.__stopTime

    def getCurrentState(self):
        return self.__currentState.value

    def getCurrentTime(self):
        return self.__currentTime

    def getCurrentStateName(self):
        return self.__currentState.name

    def getElapsed(self):
        return self.__secondsCounter

    def to_dict(self):
        return {
            'type': self.__type,
            'time': self.getCurrentTime(),
            'elapsed': self.getElapsed(),
            'event': self.__stopReason,
            'stateNum': self.getCurrentState(),
            'stateName': self.getCurrentStateName(),
            'snoozeCount': self.__snoozeCount,
            'snoozeRepetition': self.__snoozeRepetition,
            # Config information
            'NOTIFICATION_ENABLED': NotificationFSMState.NOTIFICATION_ENABLED,
            'LED_BLINK': NotificationFSMState.LED_BLINK,
            'MOTOR_VIBRATION': NotificationFSMState.MOTOR_VIBRATION,
            'SNOOZE_TIME': NotificationFSMState.SNOOZE_TIME,
            'MAX_SNOOZE': NotificationFSMState.MAX_SNOOZE,
            'PUSH_SNOOZE_COUNT': NotificationFSMState.PUSH_SNOOZE_COUNT,
            'secondsWaitingForTilt': self.__secondsWaitingForTilt
        }

    def from_dict(self, values):
        if 'type' in values and values['type'] == self.__type:
            if 'time' in values:
                self.__currentTime = values['time']

            if 'elapsed' in values:
                self.__secondsCounter = values['elapsed']

            if 'stateNum' in values:
                self.__currentState = NotificationFSMState.NotificationState(values['stateNum'])

            if 'stateName' in values:
                if self.__currentState != NotificationFSMState.NotificationState.from_name(values['stateName']):
                    print('NotificationFSMState - state mismatch')
                    return False

            if 'event' in values:
                self.__stopReason = values['event']

            # Config
            if 'NOTIFICATION_ENABLED' in values:
                NotificationFSMState.NOTIFICATION_ENABLED = values['NOTIFICATION_ENABLED']

            if 'LED_BLINK' in values:
                NotificationFSMState.LED_BLINK = values['LED_BLINK']

            if 'MOTOR_VIBRATION' in values:
                NotificationFSMState.MOTOR_VIBRATION = values['MOTOR_VIBRATION']

            if 'SNOOZE_TIME' in values:
                NotificationFSMState.SNOOZE_TIME = values['SNOOZE_TIME']

            if 'MAX_SNOOZE' in values:
                NotificationFSMState.MAX_SNOOZE = values['MAX_SNOOZE']

            return True
        return False

    def to_json(self):
        return json.dumps(self.to_dict())

    def from_json(self, data: str):
        values = json.loads(data)
        self.from_dict(values)

    def update(self, chair_state: ChairState, angle_state: AngleFSMState, travel_state: TravelFSMState,
               seating_state: SeatingFSMState, enabled=False):
        if self.__currentState == NotificationFSMState.NotificationState.INIT:
            """
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
            """
            self.__stopReason = 'Other'
            self.__snoozeCount = 0
            self.__startTime = 0
            self.__secondsCounter = 0
            self.__stopTime = 0
            self.__snoozeRepetition = 0
            self.__secondsWaitingForTilt = 0

            if seating_state.in_state(SeatingFSMState.SeatingState.CURRENTLY_SEATING):
                self.__startTime = chair_state.timestamp
                self.__currentState = NotificationFSMState.NotificationState.WAIT_PERIOD

            # Not enabled? Go back to INIT state...
            if not enabled:
                self.__currentState = NotificationFSMState.NotificationState.INIT
                self.__stopReason = 'Other'

            # no goal?
            if (angle_state.getTargetFrequency() == 0):
                self.__stopReason = "NO_GOAL"
                self.__currentState = NotificationFSMState.NotificationState.INIT

            # imu not connected ?
            if not chair_state.Angle.connectedAngle:
                self.__currentState = NotificationFSMState.NotificationState.INIT
                self.__stopReason = "notConnectedIMU"

        elif self.__currentState == NotificationFSMState.NotificationState.WAIT_PERIOD:
            """
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
            """
            self.__stopReason = 'Other'

            # Moving ?
            if travel_state.in_state(TravelFSMState.TravelState.ON_THE_MOVE):
                self.__currentState = NotificationFSMState.NotificationState.IN_TRAVEL

            # isSeated?
            if seating_state.in_state(SeatingFSMState.SeatingState.CURRENTLY_SEATING) or \
                    seating_state.in_state(SeatingFSMState.SeatingState.CONFIRM_STOP_SEATING):
                if self.__currentTime != chair_state.timestamp:
                    self.__secondsCounter += 1

                    # Waiting time for a tilt finished?
                    if ((self.__secondsCounter > angle_state.getTargetFrequency() or \
                            angle_state.in_state(AngleFSMState.AngleState.IN_TILT)) and not
                            chair_state.Angle.inCalibration):
                        self.__currentState = NotificationFSMState.NotificationState.NOTIFICATION_TILT_STARTED

            else:
                # Not seated.
                self.__currentState = NotificationFSMState.NotificationState.INIT

            # enabled?
            if not enabled:
                self.__stopReason = 'USER_DISABLED'
                self.__currentState = NotificationFSMState.NotificationState.INIT
            
            # imu not connected ?
            if not chair_state.Angle.connectedAngle:
                self.__currentState = NotificationFSMState.NotificationState.INIT
                self.__stopReason = "notConnectedIMU"

            # in calibration?
            if chair_state.Angle.inCalibration:
                self.__stopReason = "IN_CALIBRATION"
            
            # no goal?
            if (angle_state.getTargetFrequency() == 0):
                self.__stopReason = "NO_GOAL"
                self.__currentState = NotificationFSMState.NotificationState.INIT


        elif self.__currentState == NotificationFSMState.NotificationState.IN_TRAVEL:
            """
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
            """
            self.__stopReason = 'Other'
            if travel_state.in_state(TravelFSMState.TravelState.ON_THE_MOVE):
                self.__currentState = NotificationFSMState.NotificationState.IN_TRAVEL
            else:
                self.__currentState = NotificationFSMState.NotificationState.WAIT_PERIOD

            if chair_state.timestamp != self.__currentTime:
                self.__secondsCounter += 1

            # Not seated?
            if not (seating_state.in_state(SeatingFSMState.SeatingState.CURRENTLY_SEATING) or
                    seating_state.in_state(SeatingFSMState.SeatingState.CONFIRM_STOP_SEATING)):
                self.__currentState = NotificationFSMState.NotificationState.INIT

            # enabled?
            if not enabled:
                self.__stopReason = 'USER_DISABLED'
                self.__currentState = NotificationFSMState.NotificationState.INIT

            # imu not connected ?
            if not chair_state.Angle.connectedAngle:
                self.__currentState = NotificationFSMState.NotificationState.INIT
                self.__stopReason = "notConnectedIMU"

        elif self.__currentState == NotificationFSMState.NotificationState.WAITING_FOR_TILT:
            """
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

            """

            self.__stopReason = 'Other'
            self.__secondsCounter += 1
            self.__secondsWaitingForTilt += 1
            if chair_state.snoozeButton:
                self.__snoozeCount += 1 # config push_snooze_count seconds press
                if self.__snoozeCount >= NotificationFSMState.PUSH_SNOOZE_COUNT:
                    self.__snoozeRepetition += 1  
                    self.__currentState = NotificationFSMState.NotificationState.TILT_SNOOZED
                    self.__stopReason = "SNOOZED_REQUESTED"
                    self.__secondsCounter = 0
            else:
                self.__snoozeCount = 0

            # waiting period > frenquency recommended
            if (angle_state.getRecommendedTargetFrequency() != 0):
                if ((self.__secondsWaitingForTilt % angle_state.getRecommendedTargetFrequency()) == 0):
                    self.__stopReason = "MISSED_RECOMMENDED_TILT"

            # waiting period > frenquency goal
            if (angle_state.getTargetFrequency() != 0):
                if ((self.__secondsWaitingForTilt % angle_state.getTargetFrequency()) == 0):
                    self.__secondsCounter = 0
                    self.__stopReason = "MISSED_TILT"


            if travel_state.in_state(TravelFSMState.TravelState.ON_THE_MOVE):
                self.__currentState = NotificationFSMState.NotificationState.IN_TRAVEL_ELAPSED
                # TODO Best way?
                self.__currentTime = chair_state.timestamp
                return

            if not (seating_state.in_state(SeatingFSMState.SeatingState.CURRENTLY_SEATING)
                    or seating_state.in_state(SeatingFSMState.SeatingState.CONFIRM_STOP_SEATING)):
                self.__currentState = NotificationFSMState.NotificationState.NOTIFICATION_TILT_STOPPED
                self.__stopReason = 'NOT_SEATED'
                # TODO Best way?
                self.__currentTime = chair_state.timestamp
                return

            # Tilt detected?
            if ((angle_state.in_state(AngleFSMState.AngleState.ANGLE_STARTED)
                    or angle_state.in_state(AngleFSMState.AngleState.IN_TILT)) and not chair_state.Angle.inCalibration):
                self.__currentState = NotificationFSMState.NotificationState.IN_TILT
                self.__stopReason = 'TILT_BEGIN'
                self.__secondsCounter = 0
                self.__secondsWaitingForTilt = 0
            
            # in calibration?
            if chair_state.Angle.inCalibration:
                self.__stopReason = "IN_CALIBRATION"

             
            # no goal?
            if (angle_state.getTargetFrequency() == 0):
                self.__stopReason = "NO_GOAL"
                self.__currentState = NotificationFSMState.NotificationState.INIT
        
            # enabled?
            if not enabled:
                self.__stopReason = 'USER_DISABLED'
                self.__currentState = NotificationFSMState.NotificationState.INIT

            # imu not connected ?
            if not chair_state.Angle.connectedAngle:
                self.__currentState = NotificationFSMState.NotificationState.INIT
                self.__stopReason = "notConnectedIMU"

        elif self.__currentState == NotificationFSMState.NotificationState.TILT_SNOOZED:
            """
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
            """
            self.__stopReason = 'Other'
            if self.__currentTime != chair_state.timestamp:
                self.__secondsCounter += 1
                self.__secondsWaitingForTilt += 1 
                if (self.__secondsCounter >= NotificationFSMState.SNOOZE_TIME or
                        angle_state.in_state(AngleFSMState.AngleState.IN_TILT)):
                    self.__currentState = NotificationFSMState.NotificationState.WAITING_FOR_TILT
                    self.__secondsCounter = 0
                    self.__stopReason = 'TILT_REQUESTED'

            # Too much Snooze?
            if self.__snoozeRepetition > NotificationFSMState.MAX_SNOOZE:
                self.__currentState = NotificationFSMState.NotificationState.INIT
                self.__stopReason = 'TOO_MUCH_SNOOZE'

            # Not seated?
            if not (seating_state.in_state(SeatingFSMState.SeatingState.CURRENTLY_SEATING) or
                    seating_state.in_state(SeatingFSMState.SeatingState.CONFIRM_STOP_SEATING)):
                self.__currentState = NotificationFSMState.NotificationState.INIT
                self.__stopReason = 'NOT_SEATED'

            # in calibration?
            if chair_state.Angle.inCalibration:
                self.__stopReason = "IN_CALIBRATION"
            
            # no goal?
            if (angle_state.getTargetFrequency() == 0):
                self.__stopReason = "NO_GOAL"
                self.__currentState = NotificationFSMState.NotificationState.INIT

            # enabled?
            if not enabled:
                self.__stopReason = 'USER_DISABLED'
                self.__currentState = NotificationFSMState.NotificationState.INIT
            
            # imu not connected ?
            if not chair_state.Angle.connectedAngle:
                self.__currentState = NotificationFSMState.NotificationState.INIT
                self.__stopReason = "notConnectedIMU"

            # time waiting for tilit exceed the frequency recommende? 
            if (angle_state.getRecommendedTargetFrequency() != 0):
                if ((self.__secondsWaitingForTilt % angle_state.getRecommendedTargetFrequency()) == 0) :
                    self.__stopReason = "MISSED_RECOMMENDED_TILT"

            # time waiting for tilit exceed the frequency goal? 
            if (angle_state.getTargetFrequency() != 0):
                if ((self.__secondsWaitingForTilt % angle_state.getTargetFrequency()) == 0):
                    self.__stopReason = "MISSED_TILT"
                    #self.__secondsCounter = 0
                    # self.__currentState = NotificationFSMState.NotificationState.WAITING_FOR_TILT

        elif self.__currentState == NotificationFSMState.NotificationState.NOTIFICATION_TILT_STARTED:
            """
            case NotificationState::NOTIFICATION_TILT_STARTED:
                stopReason = "Other";
                currentState = NotificationState::WAITING_FOR_TILT;
                stopReason = "INITIAL_TILT_REQUESTED";
            break;
            """
            self.__currentState = NotificationFSMState.NotificationState.WAITING_FOR_TILT
            self.__secondsCounter = 0
            self.__stopReason = 'INITIAL_TILT_REQUESTED'

        elif self.__currentState == NotificationFSMState.NotificationState.IN_TILT:
            """
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
            """

            self.__stopReason = 'Other'
            # Not seated?
            if not (seating_state.in_state(SeatingFSMState.SeatingState.CURRENTLY_SEATING) or
                    seating_state.in_state(SeatingFSMState.SeatingState.CONFIRM_STOP_SEATING)):
                self.__currentState = NotificationFSMState.NotificationState.NOTIFICATION_TILT_STOPPED
                self.__stopReason = 'NOT_SEATED'
                # TODO Do better?
                self.__currentTime = chair_state.timestamp
                return

            if chair_state.timestamp != self.__currentTime:
                self.__secondsCounter += 1

            if (angle_state.in_state(AngleFSMState.AngleState.IN_TILT)
                    or angle_state.in_state(AngleFSMState.AngleState.CONFIRM_STOP_ANGLE)):
                if angle_state.getTargetDuration() <= self.__secondsCounter:
                    self.__currentState = NotificationFSMState.NotificationState.TILT_DURATION_OK
                else:
                    self.__currentState = NotificationFSMState.NotificationState.IN_TILT
            elif angle_state.in_state(AngleFSMState.AngleState.ANGLE_STOPPED):
                self.__currentState = NotificationFSMState.NotificationState.NOTIFICATION_TILT_STOPPED
                self.__stopReason = 'END_OF_TILT'
            
            # in calibration?
            if chair_state.Angle.inCalibration:
                self.__stopReason = "IN_CALIBRATION"
            
            # no goal?
            if (angle_state.getTargetFrequency() == 0):
                self.__stopReason = "NO_GOAL"
                self.__currentState = NotificationFSMState.NotificationState.INIT

            # enabled?
            if not enabled:
                self.__stopReason = 'USER_DISABLED'
                self.__currentState = NotificationFSMState.NotificationState.INIT

            # imu not connected ?
            if not chair_state.Angle.connectedAngle:
                self.__currentState = NotificationFSMState.NotificationState.INIT
                self.__stopReason = "notConnectedIMU"

        elif self.__currentState == NotificationFSMState.NotificationState.TILT_DURATION_OK:
            """
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
            """
            # Not seated?
            if not (seating_state.in_state(SeatingFSMState.SeatingState.CURRENTLY_SEATING) or
                    seating_state.in_state(SeatingFSMState.SeatingState.CONFIRM_STOP_SEATING)):
                self.__currentState = NotificationFSMState.NotificationState.NOTIFICATION_TILT_STOPPED
                self.__stopReason = 'NOT_SEATED'
                # TODO Do better?
                self.__currentTime = chair_state.timestamp
                return

            if chair_state.timestamp != self.__currentTime:
                self.__secondsCounter += 1

            if (angle_state.in_state(AngleFSMState.AngleState.IN_TILT) or
                    angle_state.in_state(AngleFSMState.AngleState.CONFIRM_STOP_ANGLE)):
                self.__currentState = NotificationFSMState.NotificationState.TILT_DURATION_OK
            elif angle_state.in_state(AngleFSMState.AngleState.ANGLE_STOPPED):
                self.__currentState = NotificationFSMState.NotificationState.NOTIFICATION_TILT_STOPPED
                self.__stopReason = 'END_OF_TILT_OK'

            # enabled?
            if not enabled:
                self.__stopReason = 'USER_DISABLED'
                self.__currentState = NotificationFSMState.NotificationState.INIT
            
            # imu not connected ?
            if not chair_state.Angle.connectedAngle:
                self.__currentState = NotificationFSMState.NotificationState.INIT
                self.__stopReason = "notConnectedIMU"

        elif self.__currentState == NotificationFSMState.NotificationState.NOTIFICATION_TILT_STOPPED:
            """
            case NotificationState::NOTIFICATION_TILT_STOPPED:
                stopReason = "NOTIFICATION_COMPLETE";
                currentState = NotificationState::INIT;
            break;
            """
            self.__stopReason = 'NOTIFICATION_COMPLETE'
            self.__currentState = NotificationFSMState.NotificationState.INIT

        elif self.__currentState == NotificationFSMState.NotificationState.IN_TRAVEL_ELAPSED:
            """
            case NotificationState::IN_TRAVEL_ELAPSED:
                if (tFSM.getCurrentState() == static_cast<int>(TravelState::ON_THE_MOVE)) {
                    currentState = NotificationState::IN_TRAVEL_ELAPSED;
                } else {
                    currentState = NotificationState::WAITING_FOR_TILT;
                }
            break;
            """
            if travel_state.in_state(TravelFSMState.TravelState.ON_THE_MOVE.value):
                self.__currentState = NotificationFSMState.NotificationState.IN_TRAVEL_ELAPSED
            else:
                self.__currentState = NotificationFSMState.NotificationState.WAITING_FOR_TILT
                self.__secondsCounter = 0
                self.__secondsWaitingForTilt = 0

        else:
            # Invalid state
            print('NotificationFSM, invalid state', self.__currentState)
            self.reset()

        # Update time
        self.__currentTime = chair_state.timestamp


class NotificationFSM:
    def __init__(self,config):
        self.state = NotificationFSMState(config)
        # Keep only state number
        self.lastNotificationState = -1
        self.chairState = ChairState()
        self.angleState = AngleFSMState(config)
        self.travelState = TravelFSMState(config)
        self.seatingState = SeatingFSMState(config)

        if config.has_section('NotificationFSM'):
            self.maxDeltaTime = config.getfloat('NotificationFSM','maxDeltaTime')
        else:
            self.maxDeltaTime = 10  # secs

    def __repr__(self):
        return "<NotificationFSM state: {}>".format(self.state.getCurrentStateName())

    def setParameters(self, enabled=False, ledBlink=False, motorVibrate=False, snoozeTime=5):
        NotificationFSMState.setParameters(enabled, ledBlink, motorVibrate, snoozeTime)

    def setChairState(self, state: ChairState):
        self.chairState = state

    def setAngeState(self, state: AngleFSMState):
        self.angleState = state

    def setTravelState(self, state: TravelFSMState):
        self.travelState = state

    def setSeatingState(self, state: SeatingFSMState):
        self.seatingState = state

    def update(self):
        # Verify timestamps
        delta_angle_time = abs(self.chairState.timestamp - self.angleState.getCurrentTime())
        delta_travel_time = abs(self.chairState.timestamp - self.travelState.getCurrentTime())
        delta_seating_time = abs(self.chairState.timestamp - self.seatingState.getCurrentTime())

        if delta_angle_time < self.maxDeltaTime and delta_travel_time < self.maxDeltaTime \
                and delta_seating_time < self.maxDeltaTime:
            self.state.update(self.chairState, self.angleState,
                              self.travelState, self.seatingState, NotificationFSMState.NOTIFICATION_ENABLED)

        else:
            print('NotificationFSM : Time mismatch angle_delta: {}, travel_delta: {}, seating_delta: {}'
                  .format(delta_angle_time, delta_travel_time, delta_seating_time))
            print('Will update later.')

    def to_json(self):
        return self.state.to_json()


async def connect_to_mqtt_server(config):
    async with AsyncExitStack() as stack:
        # Keep track of the asyncio tasks that we create, so that
        # we can cancel them on exit
        tasks = set()
        stack.push_async_callback(cancel_tasks, tasks)

        if config.has_section('MQTT'):
            # Connect to the MQTT broker
            # client = Client("10.0.1.20", username="admin", password="movitplus")
            client = Client(config.get('MQTT','broker_address'),
                            username=config.get('MQTT','usr'),
                            password=config.get('MQTT','pswd'))

            await stack.enter_async_context(client)

            # Create angle fsm
            fsm = NotificationFSM(config)

            # Messages that doesn't match a filter will get logged here
            messages = await stack.enter_async_context(client.unfiltered_messages())
            task = asyncio.create_task(log_messages(messages, "[unfiltered] {}"))
            tasks.add(task)

            # Subscribe to chairState
            await client.subscribe("sensors/chairState")
            manager = client.filtered_messages('sensors/chairState')
            messages = await stack.enter_async_context(manager)
            task = asyncio.create_task(handle_sensors_chair_state(client, messages, fsm, config))
            tasks.add(task)

            # Subscribe to angle fsm
            await client.subscribe("fsm/angle")
            manager = client.filtered_messages('fsm/angle')
            messages = await stack.enter_async_context(manager)
            task = asyncio.create_task(handle_angle_fsm_state(client, messages, fsm, config))
            tasks.add(task)

            # Subscribe to travel fsm
            await client.subscribe("fsm/travel")
            manager = client.filtered_messages('fsm/travel')
            messages = await stack.enter_async_context(manager)
            task = asyncio.create_task(handle_travel_fsm_state(client, messages, fsm, config))
            tasks.add(task)

            # Subscribe to seating fsm
            await client.subscribe("fsm/seating")
            manager = client.filtered_messages('fsm/seating')
            messages = await stack.enter_async_context(manager)
            task = asyncio.create_task(handle_seating_fsm_state(client, messages, fsm, config))
            tasks.add(task)

            # Notification changes
            await client.subscribe("config/notifications_settings")
            manager = client.filtered_messages('config/notifications_settings')
            messages = await stack.enter_async_context(manager)
            task = asyncio.create_task(handle_config_notifications_settings(client, messages, fsm))
            tasks.add(task)

            # Start periodic publish of chair state
            task = asyncio.create_task(publish_notification_fsm(client, fsm))
            tasks.add(task)

        # Wait for everything to complete (or fail due to, e.g., network errors)
        await asyncio.gather(*tasks)


async def log_messages(messages, template):
    async for message in messages:
        # 🤔 Note that we assume that the message payload is an
        # UTF8-encoded string (hence the `bytes.decode` call).
        print(template.format(message.payload.decode()))


async def cancel_tasks(tasks):
    for task in tasks:
        if task.done():
            continue
        task.cancel()
        try:
            await task
        except asyncio.CancelledError:
            pass


async def publish_notification_fsm(client, fsm: NotificationFSM):
    # 1Hz
    while True:
        # Handle FSM
        fsm.update()

        # print('publish chair state', chair_state)
        await client.publish('fsm/notification', fsm.to_json(), qos=2)

        if NotificationFSMState.NOTIFICATION_ENABLED:

            # Publish Alarm module information
            if fsm.state.in_state(NotificationFSMState.NotificationState.WAITING_FOR_TILT):
                if fsm.lastNotificationState != fsm.state.getCurrentState():
                    # Change of state
                    fsm.lastNotificationState = fsm.state.getCurrentState()
                    # TURN OFF ALARM
                    await client.publish('sensors/alarm/enabled', str(0))
                else:
                    # TURN ON ALARM
                    await client.publish('sensors/alarm/enabled', str(1))

                    if NotificationFSMState.LED_BLINK:
                        # TURN ON ALTERNATED LED BLINK
                        await client.publish('sensors/alarm/alternatingLedBlink', str(1))

                    if NotificationFSMState.MOTOR_VIBRATION:
                        # TURN ON DC MOTOR
                        await client.publish('sensors/alarm/motorOn', str(1))
                    else:
                        # TURN OFF DC MOTOR
                        await client.publish('sensors/alarm/motorOn', str(0))

            elif fsm.state.in_state(NotificationFSMState.NotificationState.IN_TILT):
                if fsm.lastNotificationState != fsm.state.getCurrentState():
                    fsm.lastNotificationState = fsm.state.getCurrentState()
                    # TURN OFF ALARM
                    await client.publish('sensors/alarm/enabled', str(0))
                else:
                    # TURN ON ALARM
                    await client.publish('sensors/alarm/enabled', str(1))

                    if fsm.chairState.Angle.seatAngle < (fsm.angleState.ANGLE_TARGET - 2):
                        # angle too low
                        if NotificationFSMState.LED_BLINK:
                            # TURN ON RED LED
                            # TURN OFF GREEN LED
                            # TURN OFF MOTOR
                            await client.publish('sensors/alarm/redLedOn', str(1))
                            await client.publish('sensors/alarm/greenLedOn', str(0))
                            await client.publish('sensors/alarm/motorOn', str(0))
                    elif fsm.chairState.Angle.seatAngle > (fsm.angleState.ANGLE_TARGET + 2):
                        # angle too high
                        if NotificationFSMState.LED_BLINK:
                            # TURN OFF RED LED
                            # TURN ON GREEN LED
                            # TURN OFF MOTOR
                            await client.publish('sensors/alarm/redLedOn', str(0))
                            await client.publish('sensors/alarm/greenLedOn', str(1))
                            await client.publish('sensors/alarm/motorOn', str(0))
                    else:
                        # angle ok
                        if NotificationFSMState.LED_BLINK:
                            # TURN ON RED LED
                            # TURN ON GREEN LED
                            # TURN OFF MOTOR
                            await client.publish('sensors/alarm/redLedOn', str(1))
                            await client.publish('sensors/alarm/greenLedOn', str(1))
                            await client.publish('sensors/alarm/motorOn', str(0))

            elif fsm.state.in_state(NotificationFSMState.NotificationState.TILT_DURATION_OK):
                if fsm.lastNotificationState != fsm.state.getCurrentState():
                    fsm.lastNotificationState = fsm.state.getCurrentState()
                    # TURN OFF ALARM
                    await client.publish('sensors/alarm/enabled', str(0))
                else:
                    # TURN ON ALARM
                    await client.publish('sensors/alarm/enabled', str(1))

                    if NotificationFSMState.LED_BLINK:
                        # TURN ON BLINK GREEN
                        await client.publish('sensors/alarm/greenLedBlink', str(1))

                    if NotificationFSMState.MOTOR_VIBRATION:
                        # TURN ON DC MOTOR
                        await client.publish('sensors/alarm/motorOn', str(1))
                    else:
                        # TURN OFF DC MOTOR
                        await client.publish('sensors/alarm/motorOn', str(0))
            else:
                # UPDATE LAST NOTIFICATION STATE
                # TURN OFF ALARM
                fsm.lastNotificationState = -1
                await client.publish('sensors/alarm/enabled', str(0))
        else:
            # TURN OFF ALARM
            fsm.lastNotificationState = -1
            await client.publish('sensors/alarm/enabled', str(0))

        # Wait next cycle
        await asyncio.sleep(1)


async def handle_sensors_chair_state(client, messages, fsm: NotificationFSM, config):
    async for message in messages:
        try:
            state = ChairState()
            state.from_json(message.payload.decode())
            fsm.setChairState(state)
        except Exception as e:
            print(e)


async def handle_config_notifications_settings(client, messages, fsm: NotificationFSM):
    async for message in messages:
        try:
            parts = message.payload.decode().split(':')
            if len(parts) == 4 and len(message.payload) > 3:
                # get all info
                leds = int(parts[0])
                vibrate = int(parts[1])
                snooze = int(parts[2])
                enabled = int(parts[3])

                # update parameters
                fsm.setParameters(enabled == 1, leds == 1, vibrate == 1, snooze)
        except Exception as e:
            print(e)


async def handle_angle_fsm_state(client, messages, fsm: NotificationFSM, config):
    async for message in messages:
        try:
            state = AngleFSMState(config)
            if state.from_json(message.payload.decode()):
                fsm.setAngeState(state)
        except Exception as e:
            print(e)


async def handle_travel_fsm_state(client, messages, fsm: NotificationFSM, config):
    async for message in messages:
        try:
            state = TravelFSMState(config)
            if state.from_json(message.payload.decode()):
                fsm.setTravelState(state)
        except Exception as e:
            print(e)


async def handle_seating_fsm_state(client, messages, fsm: NotificationFSM, config):
    async for message in messages:
        try:
            state = SeatingFSMState(config)
            if state.from_json(message.payload.decode()):
                fsm.setSeatingState(state)
        except Exception as e:
            print(e)


async def notification_fsm_main(config):
    reconnect_interval = 3  # [seconds]

    while True:
        try:
            await connect_to_mqtt_server(config)
        except MqttError as error:
            print(f'Error "{error}". Reconnecting in {reconnect_interval} seconds.')
        finally:
            await asyncio.sleep(reconnect_interval)


if __name__ == "__main__":
    # Make sure current path is this file path
    import os
    import argparse
    import configparser
    abspath = os.path.abspath(__file__)
    dname = os.path.dirname(abspath)
    os.chdir(dname)

    # Look for arguments
    argument_parser = argparse.ArgumentParser(description='NotificationFSM')
    argument_parser.add_argument('--config', type=str, help='Specify config file', default='../config.cfg')
    args = argument_parser.parse_args()


    config_parser = configparser.ConfigParser()

    print("Opening configuration file : ", args.config)
    read_ok = config_parser.read(args.config)

    if not len(read_ok):
        print('Cannot load config file', args.config)
        exit(-1)

    # main task
    asyncio.run(notification_fsm_main(config_parser))

