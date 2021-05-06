from datetime import datetime
import asyncio
import aiofiles
from contextlib import AsyncExitStack, asynccontextmanager
from asyncio_mqtt import Client, MqttError
from ChairState import ChairState, TravelInformation, AngleInformation, PressureInformation
import json
from enum import Enum, unique


class SeatingFSMState:

    SEATING_TIMEOUT = 5

    @unique
    class SeatingState(Enum):
        INIT = 0
        CONFIRM_SEATING = 1
        SEATING_STARTED = 2
        CURRENTLY_SEATING = 3
        CONFIRM_STOP_SEATING = 4
        SEATING_STOPPED = 5

        @classmethod
        def from_name(cls, name: str):
            if name == SeatingFSMState.SeatingState.INIT.name:
                return SeatingFSMState.SeatingState.INIT
            elif name == SeatingFSMState.SeatingState.CONFIRM_SEATING.name:
                return SeatingFSMState.SeatingState.CONFIRM_SEATING
            elif name == SeatingFSMState.SeatingState.SEATING_STARTED.name:
                return SeatingFSMState.SeatingState.SEATING_STARTED
            elif name == SeatingFSMState.SeatingState.CURRENTLY_SEATING.name:
                return SeatingFSMState.SeatingState.CURRENTLY_SEATING
            elif name == SeatingFSMState.SeatingState.CONFIRM_STOP_SEATING.name:
                return SeatingFSMState.SeatingState.CONFIRM_STOP_SEATING
            elif name == SeatingFSMState.SeatingState.SEATING_STOPPED.name:
                return SeatingFSMState.SeatingState.SEATING_STOPPED
            else:
                raise ValueError('{} is not a valid SeatingState name'.format(name))

    def __init__(self,config):
        self.__type = 'SeatingFSMState'
        self.__event = 'Other'
        self.__currentState = SeatingFSMState.SeatingState.INIT
        self.__seatingStarted = 0
        self.__seatingStopped = 0
        self.__currentTime = 0

        if config.has_section('SeatingFSM'):
            SeatingFSMState.SEATING_TIMEOUT = config.getfloat('SeatingFSM','SEATING_TIMEOUT')
            pass

    def in_state(self, state: SeatingState):
        return self.__currentState == state

    def getElapsedTime(self):
        return self.__seatingStopped - self.__seatingStarted

    def getStartTime(self):
        return self.__seatingStarted

    def getStopTime(self):
        return self.__seatingStopped

    def getCurrentState(self):
        return self.__currentState.value

    def getCurrentStateName(self):
        return self.__currentState.name

    def getCurrentTime(self):
        return self.__currentTime

    def to_dict(self):
        return {
            'type': self.__type,
            'time': self.__currentTime,
            'elapsed': self.getElapsedTime(),
            'event': self.__event,
            'stateNum': self.getCurrentState(),
            'stateName': self.getCurrentStateName(),
            # Config information
            'SEATING_TIMEOUT': SeatingFSMState.SEATING_TIMEOUT
        }

    def from_dict(self, values):
        if 'type' in values and values['type'] == self.__type:
            if 'time' in values:
                self.__currentTime = values['time']

            if 'elapsed' in values and 'time' in values:
                self.__seatingStopped = values['time']
                self.__seatingStarted = self.__seatingStopped = values['elapsed']

            if 'event' in values:
                self.__event = values['event']

            if 'stateNum' in values:
                self.__currentState = SeatingFSMState.SeatingState(values['stateNum'])

            if 'stateName' in values:
                if self.__currentState != SeatingFSMState.SeatingState.from_name(values['stateName']):
                    print('SeatingFSMState - state mismatch')
                    return False
            # Config
            if 'SEATING_TIMEOUT' in values:
                SeatingFSMState.SEATING_TIMEOUT = values['SEATING_TIMEOUT']

            return True
        return False

    def reset(self):
        self.__event = 'Other'
        self.__currentState = SeatingFSMState.SeatingState.INIT
        self.__seatingStarted = 0
        self.__seatingStopped = 0
        self.__currentTime = 0

    def to_json(self):
        return json.dumps(self.to_dict())

    def from_json(self, data: str):
        values = json.loads(data)
        return self.from_dict(values)

    def update(self, chair_state: ChairState):

        # Default = Other
        self.__event = 'Other'

        if self.__currentState == SeatingFSMState.SeatingState.INIT:
            """
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
            """
            if chair_state.Pressure.isSeated:
                self.__currentState = SeatingFSMState.SeatingState.CONFIRM_SEATING
                self.__seatingStarted = chair_state.timestamp
                self.__seatingStopped = chair_state.timestamp
            else:
                self.__seatingStarted = chair_state.timestamp
                self.__seatingStopped = chair_state.timestamp

        elif self.__currentState == SeatingFSMState.SeatingState.CONFIRM_SEATING:
            """
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
            """
            if not chair_state.Pressure.isSeated:
                self.__currentState = SeatingFSMState.SeatingState.INIT
            elif (chair_state.timestamp - self.__seatingStarted) > SeatingFSMState.SEATING_TIMEOUT:
                self.__seatingStarted = chair_state.timestamp
                self.__currentState = SeatingFSMState.SeatingState.SEATING_STARTED
            self.__seatingStopped = chair_state.timestamp

        elif self.__currentState == SeatingFSMState.SeatingState.SEATING_STARTED:
            """
            case SeatingState::SEATING_STARTED:
                    currentState = SeatingState::CURRENTLY_SEATING;
            break;
            """
            self.__event = 'Started'
            self.__currentState = SeatingFSMState.SeatingState.CURRENTLY_SEATING

        elif self.__currentState == SeatingFSMState.SeatingState.CURRENTLY_SEATING:
            """
            case SeatingState::CURRENTLY_SEATING:
                if(!cs.isSeated) {
                    currentState = SeatingState::CONFIRM_STOP_SEATING;
                }
                seatingStopped = cs.time;
            break;
            """
            if not chair_state.Pressure.isSeated:
                self.__currentState = SeatingFSMState.SeatingState.CONFIRM_STOP_SEATING
            self.__seatingStopped = chair_state.timestamp

        elif self.__currentState == SeatingFSMState.SeatingState.CONFIRM_STOP_SEATING:
            """
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
            """
            if chair_state.Pressure.isSeated:
                self.__currentState = SeatingFSMState.SeatingState.CURRENTLY_SEATING
                self.__seatingStopped = chair_state.timestamp
            else:
                if (chair_state.timestamp - self.__seatingStopped) > SeatingFSMState.SEATING_TIMEOUT:
                    self.__currentState = SeatingFSMState.SeatingState.SEATING_STOPPED
                    self.__seatingStopped = chair_state.timestamp

        elif self.__currentState == SeatingFSMState.SeatingState.SEATING_STOPPED:
            """
            case SeatingState::SEATING_STOPPED:
                currentState = SeatingState::INIT;
            break;
            """
            self.__event = 'Stopped'
            self.__currentState = SeatingFSMState.SeatingState.INIT
        else:
            # Invalid state
            print('SeatingFSM, invalid state', self.__currentState)
            self.reset()

        self.__currentTime = chair_state.timestamp


class SeatingFSM:
    def __init__(self,config):
        self.state = SeatingFSMState(config)
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
            fsm = SeatingFSM(config)

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
            task = asyncio.create_task(publish_seating_fsm(client, fsm))
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


async def publish_seating_fsm(client, fsm):
    # 1Hz
    while True:
        # Handle FSM
        fsm.update()

        # print('publish chair state', chair_state)
        await client.publish('fsm/seating', fsm.to_json(), qos=2)
        await asyncio.sleep(1)


async def handle_sensors_chair_state(client, messages, fsm):
    async for message in messages:
        try:
            state = ChairState()
            state.from_json(message.payload.decode())
            fsm.setChairState(state)
        except Exception as e:
            print(e)


async def seating_fsm_main(config):
    print('seating_fsm_main')
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
    argument_parser = argparse.ArgumentParser(description='SeatingFSM')
    argument_parser.add_argument('--config', type=str, help='Specify config file', default='../config.cfg')
    args = argument_parser.parse_args()


    config_parser = configparser.ConfigParser()

    print("Opening configuration file : ", args.config)
    read_ok = config_parser.read(args.config)

    if not len(read_ok):
        print('Cannot load config file', args.config)
        exit(-1)

    # main task
    asyncio.run(seating_fsm_main(config_parser))

