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
            if name == 'INIT':
                return SeatingFSMState.SeatingState.INIT
            elif name == 'CONFIRM_SEATING':
                return SeatingFSMState.SeatingState.CONFIRM_SEATING
            elif name == 'SEATING_STARTED':
                return SeatingFSMState.SeatingState.SEATING_STARTED
            elif name == 'CURRENTLY_SEATING':
                return SeatingFSMState.SeatingState.CURRENTLY_SEATING
            elif name == 'CONFIRM_STOP_SEATING':
                return SeatingFSMState.SeatingState.CONFIRM_STOP_SEATING
            elif name == 'SEATING_STOPPED':
                return SeatingFSMState.SeatingState.SEATING_STOPPED
            else:
                raise ValueError('{} is not a valid SeatingState name'.format(name))

    def __init__(self):
        self.__type = 'SeatingFSMState'
        self.__event = 'Other'
        self.__currentState = SeatingFSMState.SeatingState.INIT
        self.__seatingStarted = 0
        self.__seatingStopped = 0
        self.__currentTime = 0

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

    def to_dict(self):
        return {
            'type': self.__type,
            'time': self.__currentTime,
            'elapsed': self.getElapsedTime(),
            'event': self.__event,
            'stateNum': self.getCurrentState(),
            'stateName': self.getCurrentStateName()
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
                self.__currentState = values['stateNum']

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
        self.from_dict(values)

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
    def __init__(self):
        self.state = SeatingFSMState()
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

        if 'server' in config:
            # Connect to the MQTT broker
            # client = Client("10.0.1.20", username="admin", password="movitplus")
            client = Client(config['server']['hostname'],
                            username=config['server']['username'],
                            password=config['server']['password'])

            await stack.enter_async_context(client)

            # Create angle fsm
            fsm = SeatingFSM()

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
        state = ChairState()
        state.from_json(message.payload.decode())
        fsm.setChairState(state)


async def seating_fsm_main():
    print('seating_fsm_main')
    reconnect_interval = 3  # [seconds]

    # Read config file
    async with aiofiles.open('config.json', mode='r') as f:
        data = await f.read()
        config = json.loads(data)

    while True:
        try:
            await connect_to_mqtt_server(config)
        except MqttError as error:
            print(f'Error "{error}". Reconnecting in {reconnect_interval} seconds.')
        finally:
            await asyncio.sleep(reconnect_interval)


if __name__ == "__main__":
    # main task
    asyncio.run(seating_fsm_main())

