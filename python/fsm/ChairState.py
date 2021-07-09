
import datetime
import math
import asyncio
import aiofiles
from contextlib import AsyncExitStack, asynccontextmanager
from random import randrange
from asyncio_mqtt import Client, MqttError
import json
'''
ChairState: (sensors/chairState)
{
  "time": 1604434107,
  "snoozeButton": 0,
  "Travel": {
    "isMoving": 0,
    "lastDistance": 0
  },
  "Pressure": {
    "isSeated": 0,
    "centerOfGravity": {
      "x": 0,
      "y": 0
    },
    "centerOfGravityPerQuadrant": [
      {
        "x": 0,
        "y": 0
      },
      {
        "x": 0,
        "y": 0
      },
      {
        "x": 0,
        "y": 0
      },
      {
        "x": 0,
        "y": 0
      }
    ]
  },
  "Angle": {
    "mIMUAngle": 0,
    "fIMUAngle": 0,
    "seatAngle": 0
  }
}

topic:  config/angle_new_offset
Offset angle to be stored in the database.

'''


class TravelInformation:
    def __init__(self):
        self.sensorName = 'default'
        self.isMoving = False
        self.lastDistance = float(0.0)
        self.rawTravelData = {}

    def reset(self):
        self.sensorName = 'default'
        self.isMoving = False
        self.lastDistance = float(0.0)
        self.rawTravelData = {}

    def to_dict(self):
        return {
            'sensorName': self.sensorName,
            'isMoving': int(self.isMoving),
            'lastDistance': self.lastDistance
        }

    def from_dict(self, values: dict):
        if 'sensorName' in values:
            self.sensorName = values['sensorName']
        if 'isMoving' in values:
            self.isMoving = values['isMoving']
        if 'lastDistance' in values:
            self.lastDistance = values['lastDistance']


class PressureInformation:
    def __init__(self):
        self.sensorName = 'default'
        self.isSeated = False
        self.centerOfGravity = {'x': float(0.0), 'y': float(0.0)}
        self.centerOfGravityValue = float(0.0)
        # TODO Center of gravity per quadrant should be removed
        self.centerOfGravityQ1 = {'x': float(0.0), 'y': float(0.0)}
        self.centerOfGravityQ2 = {'x': float(0.0), 'y': float(0.0)}
        self.centerOfGravityQ3 = {'x': float(0.0), 'y': float(0.0)}
        self.centerOfGravityQ4 = {'x': float(0.0), 'y': float(0.0)}
        self.rawPressureData = {}

    def reset(self):
        self.sensorName = 'default'
        self.isSeated = False
        self.centerOfGravity = {'x': float(0.0), 'y': float(0.0)}
        self.centerOfGravityValue = float(0.0)
        # TODO Center of gravity per quadrant should be removed
        self.centerOfGravityQ1 = {'x': float(0.0), 'y': float(0.0)}
        self.centerOfGravityQ2 = {'x': float(0.0), 'y': float(0.0)}
        self.centerOfGravityQ3 = {'x': float(0.0), 'y': float(0.0)}
        self.centerOfGravityQ4 = {'x': float(0.0), 'y': float(0.0)}
        self.rawPressureData = {}

    def to_dict(self):
        return {
            'sensorName': self.sensorName,
            'isSeated': int(self.isSeated),
            'centerOfGravity': self.centerOfGravity,
            'centerOfGravityValue': self.centerOfGravityValue,
            # TODO Center of gravity per quadrant should be removed
            'centerOfGravityPerQuadrant': [self.centerOfGravityQ1, self.centerOfGravityQ2,
                                           self.centerOfGravityQ3, self.centerOfGravityQ4]
        }

    def from_dict(self, values: dict):
        if 'sensorName' in values:
            self.sensorName = values['sensorName']
        if 'isSeated' in values:
            self.isSeated = values['isSeated']
        if 'centerOfGravity' in values:
            self.centerOfGravity = values['centerOfGravity']
        if 'centerOfGravityPerQuadrant' in values:
            # TODO Center of gravity per quadrant should be removed
            if len(values['centerOfGravityPerQuadrant']) == 4:
                self.centerOfGravityQ1['x'] = values['centerOfGravityPerQuadrant'][0]['x']
                self.centerOfGravityQ1['y'] = values['centerOfGravityPerQuadrant'][0]['y']
                self.centerOfGravityQ2['x'] = values['centerOfGravityPerQuadrant'][1]['x']
                self.centerOfGravityQ2['y'] = values['centerOfGravityPerQuadrant'][1]['y']
                self.centerOfGravityQ3['x'] = values['centerOfGravityPerQuadrant'][2]['x']
                self.centerOfGravityQ3['y'] = values['centerOfGravityPerQuadrant'][2]['y']
                self.centerOfGravityQ4['x'] = values['centerOfGravityPerQuadrant'][3]['x']
                self.centerOfGravityQ4['y'] = values['centerOfGravityPerQuadrant'][3]['y']


class AngleInformation:
    def __init__(self):
        self.sensorName = 'default'
        self.stateAngle = 'default'
        self.connectedAngle = False
        self.inCalibration = False
        self.mIMUAngle = 0.0
        self.fIMUAngle = 0.0
        self.seatAngle = 0.0
        self.angleOffset = 0.0
        self.fIMURawData = {}
        self.mIMURawData = {}
        self.calibrationTime = 30
        self.calibrationEnd = 0 

    def reset(self):
        self.sensorName = 'default'
        self.stateAngle = 'default'
        self.connectedAngle = False
        self.inCalibration = False
        self.mIMUAngle = 0.0
        self.fIMUAngle = 0.0
        self.seatAngle = 0.0
        self.angleOffset = 0.0
        self.fIMURawData = {}
        self.mIMURawData = {}
        self.calibrationTime = 20;

    def to_dict(self):
        return {
            'sensorName': self.sensorName,
            'stateAngle': self.stateAngle,
            'connectedAngle': self.connectedAngle,
            'inCalibration': self.inCalibration,
            'mIMUAngle': float(format(self.mIMUAngle, '.2f')),     # float(self.mIMUAngle),
            'fIMUAngle': float(format(self.fIMUAngle, '.2f')),     # float(self.fIMUAngle),
            'seatAngle': float(format(self.seatAngle, '.2f')),     # float(self.seatAngle),
            'angleOffset': float(format(self.angleOffset, '.2f'))  # float(self.angleOffset)
        }

    def from_dict(self, values: dict):
        if 'sensorName' in values:
            self.sensorName = values['sensorName']
        if 'state' in values:
            self.stateAngle = values['stateAngle']
        if 'connectedAngle' in values:
            self.connectedAngle = values['connectedAngle']
        if 'inCalibration' in values:
            self.inCalibration = values['inCalibration']
        if 'mIMUAngle' in values:
            self.mIMUAngle = values['mIMUAngle']
        if 'fIMUAngle' in values:
            self.fIMUAngle = values['fIMUAngle']
        if 'seatAngle' in values:
            self.seatAngle = values['seatAngle']
        if 'angleOffset' in values:
            self.angleOffset = values['angleOffset']


class ChairState:
    def __init__(self):
        self.timestamp = int(datetime.datetime.now().timestamp())
        self.snoozeButton = False
        self.Travel = TravelInformation()
        self.Pressure = PressureInformation()
        self.Angle = AngleInformation()
        self.AlarmRawData = {}

    def to_dict(self):
        return {
            'time': self.timestamp,
            'snoozeButton': int(self.snoozeButton),
            'Travel': self.Travel.to_dict(),
            'Pressure': self.Pressure.to_dict(),
            'Angle': self.Angle.to_dict()
        }

    def from_dict(self, values: dict):
        if 'time' in values:
            self.timestamp = values['time']
        if 'snoozeButton' in values:
            self.snoozeButton = values['snoozeButton']
        if 'Travel' in values:
            self.Travel.from_dict(values['Travel'])
        if 'Pressure' in values:
            self.Pressure.from_dict(values['Pressure'])
        if 'Angle' in values:
            self.Angle.from_dict(values['Angle'])

    def to_json(self):
        return json.dumps(self.to_dict())

    def raw_json_data(self):
        """
        Raw Data: (sensors/rawData)
        {
            "time": 1604434062,
            "ToFSensor": {
                "connected": 0,
                "range": 260
            },
            "flowSensor": {
                "connected": 0,
                "travelX": 0,
                "travelY": 0
            },
            "alarmSensor": {
                "connected": 0,
                "redLedOn": 0,
                "redLedBlink": 0,
                "greenLedOn": 0,
                "greenLedBlink": 0,
                "alternatingLedBlink": 0,
                "motorOn": 0,
                "buttonPressed": 0
            },
            "pressureMat": {
                "threshold": 0,
                "connected": 0,
                "calibrated": 0,
                "matData": [
                0,
                0,
                0,
                0,
                0,
                0,
                0,
                0,
                0
                ]
            },
            "mIMU": {
                "connected": 0,
                "calibrated": 0,
                "accX": 0,
                "accY": 0,
                "accZ": 0,
                "gyroX": 0,
                "gyroY": 0,
                "gyroZ": 0
            },
            "fIMU": {
                "connected": 0,
                "calibrated": 0,
                "accX": 0,
                "accY": 0,
                "accZ": 0,
                "gyroX": 0,
                "gyroY": 0,
                "gyroZ": 0
            }
        }
        :param client:
        :param messages:
        :param chair_state:
        :return:
        """

        # This is for ReactUI. 
        rawData = {
            "time": int(datetime.datetime.now().timestamp()),
            "ToFSensor": self.Travel.rawTravelData,
            "flowSensor": self.Travel.rawTravelData, 
            "alarmSensor": self.AlarmRawData,
            "pressureMat": self.Pressure.rawPressureData,
            "mIMU": self.Angle.mIMURawData,
            "fIMU": self.Angle.fIMURawData
        }

        return json.dumps(rawData)    

    def from_json(self, data: str):
        values = json.loads(data)
        self.from_dict(values)

   
    def update_pressure_data(self, values: dict):
        # Reset values
        self.Pressure.reset()

        if 'name' in values:
            self.Pressure.sensorName = values['name']

        if 'cop' in values:
            self.Pressure.centerOfGravity['x'] = values['cop']['x']
            self.Pressure.centerOfGravity['y'] = values['cop']['y']
            self.Pressure.centerOfGravityValue = values['cop']['sum']

        if 'timestamp' in values:
            self.timestamp = max(values['timestamp'], self.timestamp)

        self.Pressure.rawPressureData = values

        # TODO LOOK FOR THRESHOLD
        self.Pressure.isSeated = (self.Pressure.centerOfGravityValue > 0.0)

    def update_angle_data(self, values: dict):
        # Reset values
        old_angle = self.Angle.seatAngle

        self.Angle.reset()

        if 'timestamp' in values:
            self.timestamp = max(values['timestamp'], self.timestamp)

        if 'name' in values:
            self.Angle.sensorName = values['name']
        
        if 'state' in values:
            self.Angle.stateAngle = values['state']['stateName']

        self.Angle.inCalibration = False
        if (self.Angle.stateAngle == "WAITING_FOR_CALIBRATION_TRIGGER" or self.Angle.stateAngle == "WAITING_FOR_CALIBRATION"):
            self.Angle.calibrationEnd = self.timestamp 
            self.Angle.inCalibration = True
        if ((self.timestamp - self.Angle.calibrationEnd) < self.Angle.calibrationTime):
            self.Angle.inCalibration = True
        else:
           self.Angle.calibrationEnd = 0;

        if 'connected' in values:
            self.Angle.connectedAngle = values['connected']

        if 'seat_angle' in values and self.Angle.connectedAngle:
            if not self.Travel.isMoving:
                self.Angle.seatAngle = values['seat_angle']
            else:
                self.Angle.seatAngle = old_angle

        if 'seat_angle_offset' in values and self.Angle.connectedAngle:
            self.Angle.angleOffset = values['seat_angle_offset']

        if 'mobile_imu' in values and self.Angle.connectedAngle:
            self.Angle.mIMURawData = values['mobile_imu']

        if 'fixed_imu' in values and self.Angle.connectedAngle:
            self.Angle.fIMURawData = values['fixed_imu']


    def update_travel_data(self, values: dict):
        # Reset values
        self.Travel.reset()

        self.Travel.rawTravelData = values

        if 'sensorName' in values:
            self.Travel.sensorName = values['sensorName']

        if 'timestamp' in values:
            self.timestamp = max(values['timestamp'], self.timestamp)

        connected = False
        if 'connected' in values:
            connected = values['connected']

        if 'lastDistance' in values and connected:
            self.Travel.lastDistance = values['lastDistance']

        if 'isMoving' in values and connected:
            self.Travel.isMoving = values['isMoving']

    def update_alarm_data(self, values: dict):
        connected = False
        self.snoozeButton = False

        # Keep a copy of the alarm data 
        self.AlarmRawData = values

        if 'connected' in values:
            connected = values['connected']
        if connected:
            if 'isButtonOn' in values:
                self.snoozeButton = values['isButtonOn']


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

            # Create chair state
            chair_state = ChairState()

            # Select topic filters
            # You can create any number of topic filters
            topic_filters = (
                "sensors/#",
                # TODO add more filters
            )

            # Log all messages
            # for topic_filter in topic_filters:
            #     # Log all messages that matches the filter
            #     manager = client.filtered_messages(topic_filter)
            #     messages = await stack.enter_async_context(manager)
            #     template = f'[topic_filter="{topic_filter}"] {{}}'
            #     task = asyncio.create_task(log_messages(messages, template))
            #     tasks.add(task)

            # Messages that doesn't match a filter will get logged here
            messages = await stack.enter_async_context(client.unfiltered_messages())
            task = asyncio.create_task(log_messages(messages, "[unfiltered] {}"))
            tasks.add(task)

            # Subscribe to pressure sensors
            await client.subscribe("sensors/pressure")
            manager = client.filtered_messages('sensors/pressure')
            messages = await stack.enter_async_context(manager)
            task = asyncio.create_task(handle_sensors_pressure(client, messages, chair_state))
            tasks.add(task)

            # Subscribe to angle sensors
            await client.subscribe("sensors/angle")
            manager = client.filtered_messages('sensors/angle')
            messages = await stack.enter_async_context(manager)
            task = asyncio.create_task(handle_sensors_angle(client, messages, chair_state))
            tasks.add(task)

            # Subscribe to travel sensors
            await client.subscribe("sensors/travel")
            manager = client.filtered_messages('sensors/travel')
            messages = await stack.enter_async_context(manager)
            task = asyncio.create_task(handle_sensors_travel(client, messages, chair_state))
            tasks.add(task)

            # Subscribe to alarm sensors
            await client.subscribe("sensors/alarm/state")
            manager = client.filtered_messages('sensors/alarm/state')
            messages = await stack.enter_async_context(manager)
            task = asyncio.create_task(handle_sensors_alarm(client, messages, chair_state))
            tasks.add(task)

            # Start periodic publish of chair state
            task = asyncio.create_task(publish_chair_state(client, chair_state))
            tasks.add(task)

        # Wait for everything to complete (or fail due to, e.g., network errors)
        await asyncio.gather(*tasks)


async def publish_chair_state(client, chair_state):
    # 1Hz
    while True:
        # print('publish chair state', chair_state)
        await client.publish('sensors/chairState', chair_state.to_json(), qos=2)

        # publish rawData for React UI all in one data structure 
        await client.publish('sensors/rawData', chair_state.raw_json_data(), qos=2)

        await asyncio.sleep(1)


async def log_messages(messages, template):
    async for message in messages:
        # 🤔 Note that we assume that the message paylod is an
        # UTF8-encoded string (hence the `bytes.decode` call).
        print(template.format(message.payload.decode()))


async def handle_sensors_pressure(client, messages, chair_state: ChairState):
    """

    :param client:
    :param messages:
    :param chair_state:
    :return:
    """
    async for message in messages:
        # print('pressure', message.payload.decode())
        values = json.loads(message.payload.decode())
        chair_state.update_pressure_data(values)


async def handle_sensors_angle(client, messages, chair_state: ChairState):
    """

    :param client:
    :param messages:
    :param chair_state:
    :return:
    """
    async for message in messages:
        # print('angle', message.payload.decode())
        values = json.loads(message.payload.decode())
        chair_state.update_angle_data(values)


async def handle_sensors_travel(client, messages, chair_state: ChairState):
    """

    :param client:
    :param messages:
    :param chair_state:
    :return:
    """
    async for message in messages:
        # print('travel', message.payload.decode())
        values = json.loads(message.payload.decode())
        chair_state.update_travel_data(values)


async def handle_sensors_alarm(client, messages, chair_state: ChairState):
    """

    :param client:
    :param messages:
    :param chair_state:
    :return:
    """
    async for message in messages:
        # print('alarm', message.payload.decode())
        values = json.loads(message.payload.decode())
        chair_state.update_alarm_data(values)


async def cancel_tasks(tasks):
    for task in tasks:
        if task.done():
            continue
        task.cancel()
        try:
            await task
        except asyncio.CancelledError:
            pass


async def chair_state_main(config):
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
    argument_parser = argparse.ArgumentParser(description='ChairState')
    argument_parser.add_argument('--config', type=str, help='Specify config file', default='../config.cfg')
    args = argument_parser.parse_args()


    config_parser = configparser.ConfigParser()

    print("Opening configuration file : ", args.config)
    read_ok = config_parser.read(args.config)

    if not len(read_ok):
        print('Cannot load config file', args.config)
        exit(-1)

    # main task
    asyncio.run(chair_state_main(config_parser))
