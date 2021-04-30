from datetime import datetime
import asyncio
import aiofiles
from contextlib import AsyncExitStack, asynccontextmanager
from asyncio_mqtt import Client, MqttError
from ChairState import ChairState, TravelInformation, AngleInformation, PressureInformation
import json
from enum import Enum, unique


class TravelFSMState:

    TRAVEL_START_TIMEOUT = 1
    TRAVEL_STOP_TIMEOUT = 2
    TRAVEL_THRESHOLD = 100

    @classmethod
    def travelStartTimeout(cls, timeout: int):
        TravelFSMState.TRAVEL_START_TIMEOUT = timeout

    @classmethod
    def travelStopTimeout(cls, timeout: int):
        TravelFSMState.TRAVEL_STOP_TIMEOUT = timeout

    @classmethod
    def travelThreshold(cls, threshold: int):
        TravelFSMState.TRAVEL_THRESHOLD = threshold

    @unique
    class TravelState(Enum):
        INIT = 0
        WAITING_MOVEMENT = 1
        TRAVEL_STARTED = 2
        ON_THE_MOVE = 3
        MOVEMENT_NOT_DETECTED = 4
        TRAVEL_STOPPED = 5

        @classmethod
        def from_name(cls, name: str):
            if name == TravelFSMState.TravelState.INIT.name:
                return TravelFSMState.TravelState.INIT
            elif name == TravelFSMState.TravelState.WAITING_MOVEMENT.name:
                return TravelFSMState.TravelState.WAITING_MOVEMENT
            elif name == TravelFSMState.TravelState.TRAVEL_STARTED.name:
                return TravelFSMState.TravelState.TRAVEL_STARTED
            elif name == TravelFSMState.TravelState.ON_THE_MOVE.name:
                return TravelFSMState.TravelState.ON_THE_MOVE
            elif name == TravelFSMState.TravelState.MOVEMENT_NOT_DETECTED.name:
                return TravelFSMState.TravelState.MOVEMENT_NOT_DETECTED
            elif name == TravelFSMState.TravelState.TRAVEL_STOPPED.name:
                return TravelFSMState.TravelState.TRAVEL_STOPPED
            else:
                raise ValueError('{} is not a valid TravelState name'.format(name))

    def __init__(self,config):
        self.__type = 'TravelFSMState'
        self.__event = 'Other'
        self.__currentState = TravelFSMState.TravelState.INIT
        self.__travelStartedTime = 0
        self.__travelStoppedTime = 0
        self.__lastTime = 0
        self.__travelSum = 0
        self.__currentTime = 0

        self.setConfig(config)

    def setConfig(self,config):
        TravelFSMState.TRAVEL_START_TIMEOUT = config.getfloat('TravelFSM','TRAVEL_START_TIMEOUT')
        TravelFSMState.TRAVEL_STOP_TIMEOUT = config.getfloat('TravelFSM','TRAVEL_STOP_TIMEOUT')
        TravelFSMState.TRAVEL_THRESHOLD = config.getfloat('TravelFSM','TRAVEL_THRESHOLD')

    def in_state(self, state: TravelState):
        return self.__currentState == state

    def reset(self):
        self.__event = 'Other'
        self.__currentState = TravelFSMState.TravelState.INIT
        self.__travelStartedTime = 0
        self.__travelStoppedTime = 0
        self.__lastTime = 0
        self.__travelSum = 0
        self.__currentTime = 0

    def to_dict(self):
        return {
            'type': self.__type,
            'time': self.getCurrentTime(),
            'elapsed': self.getElapsedTime(),
            'event': self.__event,
            'stateNum': self.getCurrentState(),
            'stateName': self.getCurrentStateName(),
            'travelSum': self.__travelSum,
            # Config information
            'TRAVEL_START_TIMEOUT': TravelFSMState.TRAVEL_START_TIMEOUT,
            'TRAVEL_STOP_TIMEOUT': TravelFSMState.TRAVEL_STOP_TIMEOUT,
            'TRAVEL_THRESHOLD': TravelFSMState.TRAVEL_THRESHOLD
        }

    def from_dict(self, values):
        if 'type' in values and values['type'] == self.__type:
            if 'time' in values:
                self.__currentTime = values['time']

            if 'elapsed' in values and 'time' in values:
                self.__travelStoppedTime = values['time']
                self.__travelStartedTime = values['time'] - values['elapsed']

            if 'event' in values:
                self.__event = values['event']

            if 'stateNum' in values:
                self.__currentState = TravelFSMState.TravelState(values['stateNum'])

            if 'stateName' in values:
                if self.__currentState != TravelFSMState.TravelState.from_name(values['stateName']):
                    print('TravelFSMState - state mismatch')
                    return False

            if 'travelSum' in values:
                self.__travelSum = values['travelSum']

            # Config
            if 'TRAVEL_START_TIMEOUT' in values:
                TravelFSMState.TRAVEL_START_TIMEOUT = values['TRAVEL_START_TIMEOUT']

            if 'TRAVEL_STOP_TIMEOUT' in values:
                TravelFSMState.TRAVEL_STOP_TIMEOUT = values['TRAVEL_STOP_TIMEOUT']

            if 'TRAVEL_THRESHOLD' in values:
                TravelFSMState.TRAVEL_THRESHOLD = values['TRAVEL_THRESHOLD']

            return True
        return False

    def getCurrentTime(self):
        return self.__currentTime

    def getCurrentStateName(self):
        return self.__currentState.name

    def getStartTime(self):
        return self.__travelStartedTime

    def getStopTime(self):
        return self.__travelStoppedTime

    def getCurrentState(self):
        return self.__currentState.value

    def getElapsedTime(self):
        return self.__travelStoppedTime - self.__travelStartedTime

    def to_json(self):
        return json.dumps(self.to_dict())

    def from_json(self, data: str):
        values = json.loads(data)
        return self.from_dict(values)

    def update(self, chair_state: ChairState):

        # Default = Other
        self.__event = 'Other'

        if self.__currentState == TravelFSMState.TravelState.INIT:
            """
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
            """
            # Simplified code
            if chair_state.Travel.lastDistance > TravelFSMState.TRAVEL_THRESHOLD:
                self.__currentState = TravelFSMState.TravelState.WAITING_MOVEMENT

            self.__lastTime = chair_state.timestamp
            self.__travelStartedTime = 0
            self.__travelStoppedTime = 0
            self.__travelSum = 0

        elif self.__currentState == TravelFSMState.TravelState.WAITING_MOVEMENT:
            """
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
            """
            if chair_state.timestamp - self.__lastTime > TravelFSMState.TRAVEL_START_TIMEOUT:
                if self.__travelSum > TravelFSMState.TRAVEL_THRESHOLD:
                    self.__currentState = TravelFSMState.TravelState.TRAVEL_STARTED
                    # Keep sum for next state(s)
                    # self.__travelSum = 0
                else:
                    self.__currentState = TravelFSMState.TravelState.INIT
                    # Is reinitialized in INIT
                    # self.__travelSum = 0
                self.__lastTime = chair_state.timestamp
            else:
                self.__travelSum += chair_state.Travel.lastDistance

        elif self.__currentState == TravelFSMState.TravelState.TRAVEL_STARTED:
            """
            case TravelState::TRAVEL_STARTED:
                currentState = TravelState::ON_THE_MOVE;
                travelStartedTime = cs.time;
                travelStoppedTime = cs.time;
                //printf("Travel started at: %ld\n", travelStartedTime);
            break;
            """
            self.__event = 'Started'
            self.__currentState = TravelFSMState.TravelState.ON_THE_MOVE
            self.__travelStartedTime = chair_state.timestamp
            self.__travelStoppedTime = chair_state.timestamp

        elif self.__currentState == TravelFSMState.TravelState.ON_THE_MOVE:
            """
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
            """
            if chair_state.Travel.lastDistance < 5:
                self.__lastTime = chair_state.timestamp
                # self.__travelStoppedTime = chair_state.timestamp
                self.__currentState = TravelFSMState.TravelState.MOVEMENT_NOT_DETECTED

            # Updated
            # Increment stopped time and continue accumulating distance
            self.__travelStoppedTime = chair_state.timestamp
            self.__travelSum += chair_state.Travel.lastDistance

        elif self.__currentState == TravelFSMState.TravelState.MOVEMENT_NOT_DETECTED:
            """
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
            """
            if (chair_state.timestamp - self.__lastTime) > TravelFSMState.TRAVEL_STOP_TIMEOUT:
                if chair_state.Travel.lastDistance > 25:
                    # Still moving...
                    self.__currentState = TravelFSMState.TravelState.ON_THE_MOVE
                    # Keep sum
                    # self.__travelSum = 0
                else:
                    self.__currentState = TravelFSMState.TravelState.TRAVEL_STOPPED
                    # Keep sum
                    # self.__travelSum = 0
                self.__lastTime = chair_state.timestamp

            # Keep stopped time and continue travel sum
            self.__travelSum += chair_state.Travel.lastDistance
            self.__travelStoppedTime = chair_state.timestamp

        elif self.__currentState == TravelFSMState.TravelState.TRAVEL_STOPPED:
            """
            case TravelState::TRAVEL_STOPPED:
                currentState = TravelState::INIT;
                //printf("Travel stopped at: %ld\n", travelStoppedTime);
                //printf("Moved for: %ld\n", travelStoppedTime-travelStartedTime);
            break;
            """
            self.__event = 'Stopped'
            self.__currentState = TravelFSMState.TravelState.INIT
        else:
            # Invalid state
            print('TravelFSM, invalid state', self.__currentState)
            self.reset()

        # Update time
        self.__currentTime = chair_state.timestamp


class TravelFSM:
    def __init__(self,config):
        self.state = TravelFSMState(config)
        self.chairState = ChairState()

    def setChairState(self, state: ChairState):
        self.chairState = state

    def update(self):
        self.state.update(self.chairState)

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
            fsm = TravelFSM(config)

            # Messages that doesn't match a filter will get logged here
            messages = await stack.enter_async_context(client.unfiltered_messages())
            task = asyncio.create_task(log_messages(messages, "[unfiltered] {}"))
            tasks.add(task)

            # Subscribe to chairState
            await client.subscribe("sensors/chairState")
            manager = client.filtered_messages('sensors/chairState')
            messages = await stack.enter_async_context(manager)
            task = asyncio.create_task(handle_sensors_chair_state(client, messages, fsm))
            tasks.add(task)

            # Start periodic publish of chair state
            task = asyncio.create_task(publish_travel_fsm(client, fsm))
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


async def publish_travel_fsm(client, fsm):
    # 1Hz
    while True:
        # Handle FSM
        fsm.update()

        # print('publish chair state', chair_state)
        await client.publish('fsm/travel', fsm.to_json(), qos=2)
        await asyncio.sleep(1)


async def handle_sensors_chair_state(client, messages, fsm):
    async for message in messages:
        try:
            state = ChairState()
            state.from_json(message.payload.decode())
            fsm.setChairState(state)
        except Exception as e:
            print(e)


async def travel_fsm_main(config):
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
    argument_parser = argparse.ArgumentParser(description='TravelFSM')
    argument_parser.add_argument('--config', type=str, help='Specify config file', default='../config.cfg')
    args = argument_parser.parse_args()


    config_parser = configparser.ConfigParser()

    print("Opening configuration file : ", args.config)
    read_ok = config_parser.read(args.config)

    if not len(read_ok):
        print('Cannot load config file', args.config)
        exit(-1)

    # main task
    asyncio.run(travel_fsm_main(config_parser))

