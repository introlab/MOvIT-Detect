from datetime import datetime
import asyncio
import aiofiles
from contextlib import AsyncExitStack, asynccontextmanager
from asyncio_mqtt import Client, MqttError
from ChairState import ChairState, TravelInformation, AngleInformation, PressureInformation
import json
from enum import Enum, unique

# AngleFSM: (fsm/angle)
# {
#   "time": 1604434147,
#   "elapsed": 0,
#   "event": "Other",
#   "stateNum": 0,
#   "stateName": "INIT"
# }


class AngleFSMState:

    ANGLE_TIMEOUT = 2
    ANGLE_THRESHOLD = 12
    REVERSE_ANGLE_THRESHOLD = -5
    ANGLE_TARGET = 30
    ANGLE_DURATION = 10
    ANGLE_FREQUENCY = 10

    @classmethod
    def setParameters(cls, frequency, duration, angle):
        AngleFSMState.ANGLE_TARGET = angle
        AngleFSMState.ANGLE_DURATION =  duration
        AngleFSMState.ANGLE_FREQUENCY = frequency

    @classmethod
    def getTargetAngle(cls):
        return AngleFSMState.ANGLE_TARGET

    @classmethod
    def getTargetDuration(cls):
        return AngleFSMState.ANGLE_DURATION

    @classmethod
    def getTargetFrequency(cls):
        return AngleFSMState.ANGLE_FREQUENCY

    @unique
    class AngleState(Enum):
        INIT = 0
        CONFIRM_ANGLE = 1
        ANGLE_STARTED = 2
        IN_TILT = 3
        CONFIRM_STOP_ANGLE = 4
        ANGLE_STOPPED = 5

        @classmethod
        def from_name(cls, name: str):
            if name == 'INIT':
                return AngleFSMState.AngleState.INIT
            elif name == 'CONFIRM_ANGLE':
                return AngleFSMState.AngleState.CONFIRM_ANGLE
            elif name == 'ANGLE_STARTED':
                return AngleFSMState.AngleState.ANGLE_STARTED
            elif name == 'IN_TILT':
                return AngleFSMState.AngleState.IN_TILT
            elif name == 'CONFIRM_STOP_ANGLE':
                return AngleFSMState.AngleState.CONFIRM_STOP_ANGLE
            elif name == 'ANGLE_STOPPED':
                return AngleFSMState.AngleState.ANGLE_STOPPED
            else:
                raise ValueError('{} is not a valid AngleState name'.format(name))

    def __init__(self):
        self.__type = 'AngleFSMState'
        self.__state = AngleFSMState.AngleState.INIT
        self.__event = 'Other'
        # internal variables
        self.__result = [0, 0, 0, 0, 0]
        self.__lastTime = 0
        self.__sum = 0
        self.__dataPoints = 0
        self.__currentTime = 0
        self.__angleStarted = 0
        self.__angleStopped = 0

    def getStartTime(self):
        return self.__angleStarted

    def getStopTime(self):
        return self.__angleStopped

    def getCurrentState(self):
        return self.__state.value

    def getElapsedTime(self):
        return self.__angleStopped - self.__angleStarted

    def getCurrentStateName(self):
        return self.__state.name

    def getCurrentTime(self):
        return self.__currentTime

    def getTimePerAngle(self):
        return self.__result

    def getAngleAverage(self):
        if self.__dataPoints != 0:
            return self.__sum / self.__dataPoints
        else:
            return 0

    def to_dict(self):
        return {
            'type': self.__type,
            'time': self.getCurrentTime(),
            'elapsed': self.getElapsedTime(),
            'event': self.__event,
            'stateNum': self.getCurrentState(),
            'stateName': self.getCurrentStateName(),
            # Additional fields (always output?)
            'lessThanZero': self.__result[0],
            'zeroToFifteen': self.__result[1],
            'fifteenToThirty': self.__result[2],
            'thirtyToFourtyfive': self.__result[3],
            'fourtyfiveAndMore': self.__result[4],
            'average': self.getAngleAverage(),
            'requiredDuration': self.getTargetDuration(),
            'requiredAngle': self.getTargetAngle()
        }

    def from_dict(self, values: dict):
        if 'type' in values and values['type'] == self.__type:
            if 'time' in values:
                self.__currentTime = values['time']

            if 'elapsed' in values:
                self.__angleStarted = self.__currentTime - values['elapsed']
                self.__angleStopped = self.__currentTime

            if 'event' in values:
                self.__event = values['event']

            if 'stateNum' in values:
                self.__state = AngleFSMState.AngleState(values['stateNum'])

            if 'stateName' in values:
                if self.__state != AngleFSMState.AngleState.from_name(values['stateName']):
                    print('State mismatch')
            return True
        return False

    def reset(self):
        self.__state = AngleFSMState.AngleState.INIT
        self.__event = 'Other'
        # internal variables
        self.__result = [0, 0, 0, 0, 0]
        self.__lastTime = 0
        self.__sum = 0
        self.__dataPoints = 0
        self.__currentTime = 0
        self.__angleStarted = 0
        self.__angleStopped = 0

    def to_json(self):
        return json.dumps(self.to_dict())

    def from_json(self, data: str):
        values = json.loads(data)
        self.from_dict(values)

    def update(self, chair_state: ChairState):
        print('update')

        # Default = Other
        self.__event = 'Other'

        if self.__state == AngleFSMState.AngleState.INIT:
            """
               case AngleState::INIT:
                    if (cs.seatAngle >= ANGLE_THRESHOLD || cs.seatAngle <= REVERSE_ANGLE_THRESHOLD)
                    {
                        currentState = AngleState::CONFIRM_ANGLE;
                        angleStarted = cs.time;
                        angleStopped = cs.time;
                        result[0] = 0;
                        result[1] = 0;
                        result[2] = 0;
                        result[3] = 0;
                        result[4] = 0;
                        sum = 0;
                        dataPoints = 0;
                    } else {
                        angleStarted = 0;
                        angleStopped = 0;
                    }
                break;
            """
            if chair_state.Angle.seatAngle >= AngleFSMState.ANGLE_THRESHOLD \
                    or chair_state.Angle.seatAngle <= AngleFSMState.REVERSE_ANGLE_THRESHOLD:
                self.__state = AngleFSMState.AngleState.CONFIRM_ANGLE
                self.__angleStarted = chair_state.timestamp
                self.__angleStopped = chair_state.timestamp
                self.__result = [0, 0, 0, 0, 0]
                self.__sum = 0
                self.__dataPoints = 0
            else:
                self.__angleStarted = 0
                self.__angleStopped = 0

        elif self.__state == AngleFSMState.AngleState.CONFIRM_ANGLE:
            """
                 case AngleState::CONFIRM_ANGLE:
                    if (cs.seatAngle < ANGLE_THRESHOLD && cs.seatAngle > REVERSE_ANGLE_THRESHOLD)
                    {
                        currentState = AngleState::INIT;
                        angleStarted = 0;
                    } else if((cs.time - angleStarted) > ANGLE_TIMEOUT) {
                        angleStarted = cs.time;
                        currentState = AngleState::ANGLE_STARTED;
                    }
                    angleStopped = cs.time;
                break;
            """
            if AngleFSMState.ANGLE_THRESHOLD > chair_state.Angle.seatAngle > AngleFSMState.REVERSE_ANGLE_THRESHOLD:
                self.__state = AngleFSMState.AngleState.INIT
                self.__angleStarted = 0
            elif (chair_state.timestamp - self.__angleStarted) > self.ANGLE_TIMEOUT:
                self.__angleStarted = chair_state.timestamp
                self.__state = AngleFSMState.AngleState.ANGLE_STARTED

            self.__angleStopped = chair_state.timestamp

        elif self.__state == AngleFSMState.AngleState.ANGLE_STARTED:
            """
            case AngleState::ANGLE_STARTED:
                angleStarted = cs.time;
                currentState = AngleState::IN_TILT;
            break;
            """
            self.__event = 'Started'
            self.__angleStarted = chair_state.timestamp
            self.__angleStopped = chair_state.timestamp
            self.__state = AngleFSMState.AngleState.IN_TILT
        elif self.__state == AngleFSMState.AngleState.IN_TILT:
            """
            case AngleState::IN_TILT:
                if ((cs.time - lastTime) >= 1)
                {
                    if (cs.seatAngle < 0)
                    {
                        result[0]++;
                    }
                    else if (cs.seatAngle >= 0 && cs.seatAngle < 15)
                    {
                        result[1]++;
                    }
                    else if (cs.seatAngle >= 15 && cs.seatAngle < 30)
                    {
                        result[2]++;
                    }
                    else if (cs.seatAngle >= 30 && cs.seatAngle < 45)
                    {
                        result[3]++;
                    }
                    else
                    {
                        result[4]++;
                    }
                    lastTime = cs.time;
                }
    
                dataPoints++;
                sum += cs.seatAngle;
    
                if(cs.seatAngle < ANGLE_THRESHOLD && cs.seatAngle > REVERSE_ANGLE_THRESHOLD) {
                    angleStopped = cs.time;
                    currentState = AngleState::CONFIRM_STOP_ANGLE;
                }
                angleStopped = cs.time;
            break;
            """
            if chair_state.timestamp - self.__lastTime >= 1:
                if chair_state.Angle.seatAngle < 0:
                    self.__result[0] += 1
                elif 0 <= chair_state.Angle.seatAngle < 15:
                    self.__result[1] += 1
                elif 15 <= chair_state.Angle.seatAngle < 30:
                    self.__result[2] += 1
                elif 30 <= chair_state.Angle.seatAngle < 45:
                    self.__result[3] += 1
                else:
                    self.__result[4] += 1

                self.__lastTime = chair_state.timestamp

            self.__dataPoints += 1
            self.__sum += chair_state.Angle.seatAngle

            if AngleFSMState.ANGLE_THRESHOLD > chair_state.Angle.seatAngle > AngleFSMState.REVERSE_ANGLE_THRESHOLD:
                self.__angleStopped = chair_state.timestamp
                self.__state = AngleFSMState.AngleState.CONFIRM_STOP_ANGLE

            self.__angleStopped = chair_state.timestamp

        elif self.__state == AngleFSMState.AngleState.CONFIRM_STOP_ANGLE:
            """
            case AngleState::CONFIRM_STOP_ANGLE:
                if(cs.seatAngle >= ANGLE_THRESHOLD || cs.seatAngle <= REVERSE_ANGLE_THRESHOLD) {
                    currentState = AngleState::IN_TILT;
                    angleStopped = 0;
                } else {
                    if((cs.time - angleStopped) > ANGLE_TIMEOUT) {
                        currentState = AngleState::ANGLE_STOPPED;
                    }
                }
            break;
            """
            if chair_state.Angle.seatAngle >= AngleFSMState.ANGLE_THRESHOLD or \
                    chair_state.Angle.seatAngle <= AngleFSMState.REVERSE_ANGLE_THRESHOLD:
                self.__state = AngleFSMState.AngleState.IN_TILT
                # TODO Verify 0
                self.__angleStopped = 0
            else:
                # Wait for timeout
                if (chair_state.timestamp - self.__angleStopped) > AngleFSMState.ANGLE_TIMEOUT:
                    self.__state = AngleFSMState.AngleState.ANGLE_STOPPED

        elif self.__state == AngleFSMState.AngleState.ANGLE_STOPPED:
            """
            case AngleState::ANGLE_STOPPED:
                currentState = AngleState::INIT;
            break;
            """
            self.__event = 'Stopped'
            self.__state = AngleFSMState.AngleState.INIT
        else:
            print('AngleFSM, invalid state', self.__state)
            self.reset()

        self.__currentTime = chair_state.timestamp


class AngleFSM:
    def __init__(self):
        self.state = AngleFSMState()
        self.chairState = ChairState()

    def setChairState(self, state: ChairState):
        self.chairState = state

    def to_json(self):
        return self.state.to_json()

    def update(self):
        # Update state according to chair state
        self.state.update(self.chairState)


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
            fsm = AngleFSM()

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
            task = asyncio.create_task(publish_angle_fsm(client, fsm))
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


async def publish_angle_fsm(client, fsm):
    # 1Hz
    while True:
        # Handle FSM
        fsm.update()

        # print('publish chair state', chair_state)
        await client.publish('fsm/angle', fsm.to_json(), qos=2)
        await asyncio.sleep(1)


async def handle_sensors_chair_state(client, messages, fsm):
    async for message in messages:
        state = ChairState()
        state.from_json(message.payload.decode())
        fsm.setChairState(state)


async def angle_fsm_main():
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
    asyncio.run(angle_fsm_main())
