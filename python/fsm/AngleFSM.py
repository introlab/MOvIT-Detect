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
    ANGLE_RECOMMENDED_TARGET = ANGLE_TARGET
    ANGLE_RECOMMENDED_DURATION = ANGLE_DURATION
    ANGLE_RECOMMENDED_FREQUENCY = ANGLE_FREQUENCY

    ANGLE_TILT_0 = 0
    ANGLE_TILT_1 = 15
    ANGLE_TILT_2 = 30
    ANGLE_TILT_3 = 45

    @classmethod
    def setParameters(cls, frequency, duration, angle):
        AngleFSMState.ANGLE_TARGET = angle
        AngleFSMState.ANGLE_DURATION = duration
        AngleFSMState.ANGLE_FREQUENCY = frequency

    @classmethod
    def setRecommendedParameters(cls, frequency, duration, angle):
        AngleFSMState.ANGLE_RECOMMENDED_TARGET = angle
        AngleFSMState.ANGLE_RECOMMENDED_DURATION = duration
        AngleFSMState.ANGLE_RECOMMENDED_FREQUENCY = frequency

    @classmethod
    def getTargetAngle(cls):
        return AngleFSMState.ANGLE_TARGET

    @classmethod
    def getTargetDuration(cls):
        return AngleFSMState.ANGLE_DURATION

    @classmethod
    def getTargetFrequency(cls):
        return AngleFSMState.ANGLE_FREQUENCY
    
    @classmethod
    def getRecommendedTargetAngle(cls):
        return AngleFSMState.ANGLE_RECOMMENDED_TARGET

    @classmethod
    def getRecommendedTargetDuration(cls):
        return AngleFSMState.ANGLE_RECOMMENDED_DURATION

    @classmethod
    def getRecommendedTargetFrequency(cls):
        return AngleFSMState.ANGLE_RECOMMENDED_FREQUENCY

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
            if name == AngleFSMState.AngleState.INIT.name:
                return AngleFSMState.AngleState.INIT
            elif name == AngleFSMState.AngleState.CONFIRM_ANGLE.name:
                return AngleFSMState.AngleState.CONFIRM_ANGLE
            elif name == AngleFSMState.AngleState.ANGLE_STARTED.name:
                return AngleFSMState.AngleState.ANGLE_STARTED
            elif name == AngleFSMState.AngleState.IN_TILT.name:
                return AngleFSMState.AngleState.IN_TILT
            elif name == AngleFSMState.AngleState.CONFIRM_STOP_ANGLE.name:
                return AngleFSMState.AngleState.CONFIRM_STOP_ANGLE
            elif name == AngleFSMState.AngleState.ANGLE_STOPPED.name:
                return AngleFSMState.AngleState.ANGLE_STOPPED
            else:
                raise ValueError('{} is not a valid AngleState name'.format(name))

    def __init__(self,config):
        self.__type = 'AngleFSMState'
        self.__currentState = AngleFSMState.AngleState.INIT
        self.__event = 'Other'
        # internal variables
        self.__result = [0, 0, 0, 0, 0]
        self.__lastTime = 0
        self.__sum = 0
        self.__dataPoints = 0
        self.__currentTime = 0
        self.__angleStarted = 0
        self.__angleStopped = 0

        if config.has_section('AngleFSM'):
            AngleFSMState.ANGLE_TIMEOUT = config.getfloat('AngleFSM','ANGLE_TIMEOUT')
            AngleFSMState.ANGLE_THRESHOLD = config.getfloat('AngleFSM','ANGLE_THRESHOLD')
            AngleFSMState.REVERSE_ANGLE_THRESHOLD = config.getfloat('AngleFSM','REVERSE_ANGLE_THRESHOLD')
            AngleFSMState.ANGLE_TILT_0 = config.getfloat('AngleFSM','ANGLE_TILT_0')
            AngleFSMState.ANGLE_TILT_1 = config.getfloat('AngleFSM','ANGLE_TILT_1')
            AngleFSMState.ANGLE_TILT_2 = config.getfloat('AngleFSM','ANGLE_TILT_2')
            AngleFSMState.ANGLE_TILT_3 = config.getfloat('AngleFSM','ANGLE_TILT_3')

    def reset(self):
        self.__currentState = AngleFSMState.AngleState.INIT
        self.__event = 'Other'
        # internal variables
        self.__result = [0, 0, 0, 0, 0]
        self.__lastTime = 0
        self.__sum = 0
        self.__dataPoints = 0
        self.__currentTime = 0
        self.__angleStarted = 0
        self.__angleStopped = 0

    def in_state(self, state: AngleState):
        return self.__currentState == state

    def getStartTime(self):
        return self.__angleStarted

    def getStopTime(self):
        return self.__angleStopped

    def getCurrentState(self):
        return self.__currentState.value

    def getElapsedTime(self):
        return self.__angleStopped - self.__angleStarted

    def getCurrentStateName(self):
        return self.__currentState.name

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
            'average': float(format(self.getAngleAverage(), '.2f')),
            'requiredDuration': self.getTargetDuration(),
            'requiredAngle': self.getTargetAngle(),
            # Config information
            'ANGLE_TIMEOUT': AngleFSMState.ANGLE_TIMEOUT,
            'ANGLE_THRESHOLD': AngleFSMState.ANGLE_THRESHOLD,
            'REVERSE_ANGLE_THRESHOLD': AngleFSMState.REVERSE_ANGLE_THRESHOLD,
            'ANGLE_TARGET': AngleFSMState.ANGLE_TARGET,
            'ANGLE_DURATION': AngleFSMState.ANGLE_DURATION,
            'ANGLE_FREQUENCY': AngleFSMState.ANGLE_FREQUENCY,
            'ANGLE_RECOMMENDED_TARGET': AngleFSMState.ANGLE_RECOMMENDED_TARGET,
            'ANGLE_RECOMMENDED_DURATION': AngleFSMState.ANGLE_RECOMMENDED_DURATION,
            'ANGLE_RECOMMENDED_FREQUENCY': AngleFSMState.ANGLE_RECOMMENDED_FREQUENCY
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
                self.__currentState = AngleFSMState.AngleState(values['stateNum'])

            if 'stateName' in values:
                if self.__currentState != AngleFSMState.AngleState.from_name(values['stateName']):
                    print('AngleFSMState - state mismatch')
                    return False

            # Config
            if 'ANGLE_TIMEOUT' in values:
                AngleFSMState.ANGLE_TIMEOUT = values['ANGLE_TIMEOUT']

            if 'ANGLE_THRESHOLD' in values:
                AngleFSMState.ANGLE_THRESHOLD = values['ANGLE_THRESHOLD']

            if 'REVERSE_ANGLE_THRESHOLD' in values:
                AngleFSMState.REVERSE_ANGLE_THRESHOLD = values['REVERSE_ANGLE_THRESHOLD']

            if 'ANGLE_TARGET' in values:
                AngleFSMState.ANGLE_TARGET = values['ANGLE_TARGET']

            if 'ANGLE_DURATION' in values:
                AngleFSMState.ANGLE_DURATION = values['ANGLE_DURATION']

            if 'ANGLE_FREQUENCY' in values:
                AngleFSMState.ANGLE_FREQUENCY = values['ANGLE_FREQUENCY']

            if 'ANGLE_RECOMMENDED_TARGET' in values:
                AngleFSMState.ANGLE_RECOMMENDED_TARGET = values['ANGLE_RECOMMENDED_TARGET']

            if 'ANGLE_RECOMMENDED_DURATION' in values:
                AngleFSMState.ANGLE_RECOMMENDED_DURATION = values['ANGLE_RECOMMENDED_DURATION']

            if 'ANGLE_RECOMMENDED_FREQUENCY' in values:
                AngleFSMState.ANGLE_RECOMMENDED_FREQUENCY = values['ANGLE_RECOMMENDED_FREQUENCY']


            return True
        return False

    def to_json(self):
        return json.dumps(self.to_dict())

    def from_json(self, data: str):
        values = json.loads(data)
        return self.from_dict(values)

    def update(self, chair_state: ChairState):
        # Default = Other
        self.__event = 'Other'

        if self.__currentState == AngleFSMState.AngleState.INIT:
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

            # Init default values
            self.__angleStarted = chair_state.timestamp
            self.__angleStopped = chair_state.timestamp
            self.__result = [0, 0, 0, 0, 0]
            self.__sum = 0
            self.__dataPoints = 0

            if (chair_state.Angle.seatAngle >= AngleFSMState.ANGLE_THRESHOLD \
                    or chair_state.Angle.seatAngle <= AngleFSMState.REVERSE_ANGLE_THRESHOLD) and not chair_state.Travel.isMoving and chair_state.Pressure.isSeated:
                # Change state
                self.__currentState = AngleFSMState.AngleState.CONFIRM_ANGLE

        elif self.__currentState == AngleFSMState.AngleState.CONFIRM_ANGLE:
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
            if (AngleFSMState.ANGLE_THRESHOLD > chair_state.Angle.seatAngle > AngleFSMState.REVERSE_ANGLE_THRESHOLD) or chair_state.Travel.isMoving or not chair_state.Pressure.isSeated:
                # Go back to INIT state
                self.__currentState = AngleFSMState.AngleState.INIT
            elif (chair_state.timestamp - self.__angleStarted) > AngleFSMState.ANGLE_TIMEOUT:
                self.__angleStarted = chair_state.timestamp
                # Go to ANGLE_STARTED
                self.__currentState = AngleFSMState.AngleState.ANGLE_STARTED

            self.__angleStopped = chair_state.timestamp

        elif self.__currentState == AngleFSMState.AngleState.ANGLE_STARTED:
            """
            case AngleState::ANGLE_STARTED:
                angleStarted = cs.time;
                currentState = AngleState::IN_TILT;
            break;
            """
            self.__event = 'Started'
            self.__angleStarted = chair_state.timestamp
            self.__angleStopped = chair_state.timestamp
            self.__currentState = AngleFSMState.AngleState.IN_TILT
        elif self.__currentState == AngleFSMState.AngleState.IN_TILT:
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
                if chair_state.Angle.seatAngle < AngleFSMState.ANGLE_TILT_0:
                    self.__result[0] += 1
                elif AngleFSMState.ANGLE_TILT_0 <= chair_state.Angle.seatAngle < AngleFSMState.ANGLE_TILT_1:
                    self.__result[1] += 1
                elif AngleFSMState.ANGLE_TILT_1 <= chair_state.Angle.seatAngle < AngleFSMState.ANGLE_TILT_2:
                    self.__result[2] += 1
                elif AngleFSMState.ANGLE_TILT_2 <= chair_state.Angle.seatAngle < AngleFSMState.ANGLE_TILT_3:
                    self.__result[3] += 1
                else:
                    self.__result[4] += 1

                self.__lastTime = chair_state.timestamp

            self.__dataPoints += 1
            self.__sum += chair_state.Angle.seatAngle

            if (AngleFSMState.ANGLE_THRESHOLD > chair_state.Angle.seatAngle > AngleFSMState.REVERSE_ANGLE_THRESHOLD) or chair_state.Travel.isMoving or not chair_state.Pressure.isSeated:
                self.__currentState = AngleFSMState.AngleState.CONFIRM_STOP_ANGLE

            self.__angleStopped = chair_state.timestamp

        elif self.__currentState == AngleFSMState.AngleState.CONFIRM_STOP_ANGLE:
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
            if chair_state.Travel.isMoving or not chair_state.Pressure.isSeated:
                self.__currentState = AngleFSMState.AngleState.ANGLE_STOPPED
            else:
                # Do not modify angle started or angle stopped here
                if chair_state.Angle.seatAngle >= AngleFSMState.ANGLE_THRESHOLD or \
                        chair_state.Angle.seatAngle <= AngleFSMState.REVERSE_ANGLE_THRESHOLD:
                    self.__currentState = AngleFSMState.AngleState.IN_TILT
                else:
                    # Wait for timeout
                    if (chair_state.timestamp - self.__angleStopped) > AngleFSMState.ANGLE_TIMEOUT:
                        self.__currentState = AngleFSMState.AngleState.ANGLE_STOPPED

        elif self.__currentState == AngleFSMState.AngleState.ANGLE_STOPPED:
            """
            case AngleState::ANGLE_STOPPED:
                currentState = AngleState::INIT;
            break;
            """
            self.__event = 'Stopped'
            self.__currentState = AngleFSMState.AngleState.INIT
        else:
            print('AngleFSM, invalid state', self.__currentState)
            self.reset()

        self.__currentTime = chair_state.timestamp


class AngleFSM:
    def __init__(self,config):
        self.state = AngleFSMState(config)
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

        if config.has_section('MQTT'):
            # Connect to the MQTT broker
            # client = Client("10.0.1.20", username="admin", password="movitplus")
            client = Client(config.get('MQTT','broker_address'),
                            username=config.get('MQTT','usr'),
                            password=config.get('MQTT','pswd'))

            await stack.enter_async_context(client)

            # Create angle fsm
            fsm = AngleFSM(config)

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

            # Subscribe to config notification settings changes
            await client.subscribe("goal/update_data")
            manager = client.filtered_messages('goal/update_data')
            messages = await stack.enter_async_context(manager)
            task = asyncio.create_task(handle_goal_update_data(client, messages, fsm))
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


async def handle_goal_update_data(client, messages, fsm):
    async for message in messages:
        try:
            # DL - 27/05/2021 new goal as user / clinician json 
            reqs = json.loads(message.payload)

            if 'user' in reqs:
                frequency = reqs['user']['tiltFrequency']
                duration = reqs['user']['tiltLength']
                angle = reqs['user']['tiltAngle']

                # update parameters
                AngleFSMState.setParameters(frequency, duration, angle)

            if 'clinician' in reqs:
                frequency = reqs['clinician']['tiltFrequency']
                duration = reqs['clinician']['tiltLength']
                angle = reqs['clinician']['tiltAngle']

                # update parameters
                AngleFSMState.setRecommendedParameters(frequency, duration, angle)


        except Exception as e:
            print(e)


async def handle_sensors_chair_state(client, messages, fsm):
    async for message in messages:
        try:
            state = ChairState()
            state.from_json(message.payload.decode())
            fsm.setChairState(state)
        except Exception as e:
            print(e)


async def angle_fsm_main(config):
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
    argument_parser = argparse.ArgumentParser(description='AngleFSM')
    argument_parser.add_argument('--config', type=str, help='Specify config file', default='../config.cfg')
    args = argument_parser.parse_args()


    config_parser = configparser.ConfigParser()

    print("Opening configuration file : ", args.config)
    read_ok = config_parser.read(args.config)

    if not len(read_ok):
        print('Cannot load config file', args.config)
        exit(-1)

    # main task
    asyncio.run(angle_fsm_main(config_parser))

    
