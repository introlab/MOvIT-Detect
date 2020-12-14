
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

    def reset(self):
        self.sensorName = 'default'
        self.isMoving = False
        self.lastDistance = float(0.0)

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
        self.mIMUAngle = 0.0
        self.fIMUAngle = 0.0
        self.seatAngle = 0.0
        self.angleOffset = 0.0

    def reset(self):
        self.sensorName = 'default'
        self.mIMUAngle = 0.0
        self.fIMUAngle = 0.0
        self.seatAngle = 0.0
        self.angleOffset = 0.0

    def to_dict(self):
        return {
            'sensorName': self.sensorName,
            'mIMUAngle': float(format(self.mIMUAngle, '.2f')),     # float(self.mIMUAngle),
            'fIMUAngle': float(format(self.fIMUAngle, '.2f')),     # float(self.fIMUAngle),
            'seatAngle': float(format(self.seatAngle, '.2f')),     # float(self.seatAngle),
            'angleOffset': float(format(self.angleOffset, '.2f'))  # float(self.angleOffset)
        }

    def from_dict(self, values: dict):
        if 'sensorName' in values:
            self.sensorName = values['sensorName']
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

        # const Coord_t POSITION_LOOKUP[9] = {{4.0f,4.0f}, {4.0f,0.0f}, {4.0f,-4.0f},
        # {0.0f,4.0f}, {0.0f,0.0f}, {0.0f,-4.0f}, {-4.0f,4.0f}, {-4.0f,0.0f}, {-4.0f,-4.0f}};

        # This should be modified with the type of pressure mat sensor
        # units are in centimeters
        # (0,0) is center of seat
        self.positionLookup = [
            {'x': 4.0, 'y': 4.0},
            {'x': 4.0, 'y': 0.0},
            {'x': 4.0, 'y': -4.0},
            {'x': 0.0, 'y': 4.0},
            {'x': 0.0, 'y': 0.0},
            {'x': 0.0, 'y': -4.0},
            {'x': -4.0, 'y': 4.0},
            {'x': -4.0, 'y': 0.0},
            {'x': -4.0, 'y': -4.0}
        ]

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

    def from_json(self, data: str):
        values = json.loads(data)
        self.from_dict(values)

    def calculate_fIMU_angle(self, acc_x, acc_y, acc_z):
        # Old implementation to be replaced by something better
        y = -acc_x
        x = math.sqrt(acc_y * acc_y + acc_z * acc_z)
        # Convert radians to degrees
        return math.atan2(y, x) * (180.0 / math.pi)

    def calculate_mIMU_angle(self, acc_x, acc_y, acc_z):
        # Old implementation to be replaced by something better
        y = -acc_x
        x = math.sqrt(acc_y * acc_y + acc_z * acc_z)
        # Convert radians to degrees
        return math.atan2(y, x) * (180.0 / math.pi)

    def calculate_9x9_center_of_pressure(self, data: list, coordinates: list, threshold=0.0):
        result = {'x': 0.0, 'y': 0.0, 'value': 0.0, 'isSeated': False}

        # Make sure we have the right vector length
        if len(data) == len(coordinates):
            s = 0
            position = {'x': 0.0, 'y': 0.0}

            for i in range(len(data)):
                position['x'] += data[i] * coordinates[i]['x']
                position['y'] += data[i] * coordinates[i]['y']
                s += data[i]

            if s == 0:
                position['x'] = 0.0
                position['y'] = 0.0
            else:
                position['x'] = position['x'] / s
                position['y'] = position['y'] / s

            result['x'] = position['x']
            result['y'] = position['y']
            # Sum of all sensors
            result['value'] = s

            # TODO isSeated
            if result['value'] > threshold:
                result['isSeated'] = True

        return result

    def updateRawData(self, values: dict):
        """
            The data comes from the old C implementation. Let's calculate everything all at once.
        """

        # Reset everything
        self.Pressure.reset()
        self.Angle.reset()
        self.Travel.reset()

        if 'time' in values:
            self.timestamp = values['time']

        # Mobile IMU
        if 'mIMU' in values and values['mIMU']['connected']:
            self.Angle.mIMUAngle = self.calculate_mIMU_angle(values['mIMU']['accX'],
                                                             values['mIMU']['accY'],
                                                             values['mIMU']['accZ'])

        # Fixed IMU
        if 'fIMU' in values and values['fIMU']['connected']:
            self.Angle.fIMUAngle = self.calculate_fIMU_angle(values['fIMU']['accX'],
                                                             values['fIMU']['accY'],
                                                             values['fIMU']['accZ'])

        # Angle calculation
        self.Angle.seatAngle = - (self.Angle.fIMUAngle - self.Angle.mIMUAngle - self.Angle.angleOffset)

        if 'pressureMat' in values and 'matData' in values['pressureMat']:
            # Calculate center of pressure
            cop = self.calculate_9x9_center_of_pressure(values['pressureMat']['matData'],
                                                        self.positionLookup,
                                                        threshold=values['pressureMat']['threshold'])
            self.Pressure.isSeated = cop['isSeated']
            self.Pressure.centerOfGravity['x'] = cop['x']
            self.Pressure.centerOfGravity['y'] = cop['y']
            self.Pressure.centerOfGravityValue = cop['value']
            # TODO Center of gravity per quadrant should be removed.

        # Travel calculation
        if 'flowSensor' in values and values['flowSensor']['connected']:
            self.Travel.lastDistance += math.sqrt(math.pow(values['flowSensor']['travelX'], 2) +
                                                  math.pow(values['flowSensor']['travelY'], 2))

            # Are we moving?
            # TODO hard coded threshold here...
            if self.Travel.lastDistance > 25.0:
                self.Travel.isMoving = True

        # Snooze button
        if 'alarmSensor' in values and values['alarmSensor']['connected']:
            self.snoozeButton = values['alarmSensor']['buttonPressed']

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

        # TODO LOOK FOR THRESHOLD
        self.Pressure.isSeated = (self.Pressure.centerOfGravityValue > 0.0)

    def update_angle_data(self, values: dict):
        # Reset values
        self.Angle.reset()

        if 'name' in values:
            self.Angle.sensorName = values['name']

        if 'timestamp' in values:
            self.timestamp = max(values['timestamp'], self.timestamp)

        connected = False
        if 'connected' in values:
            connected = values['connected']

        if 'seat_angle' in values and connected:
            self.Angle.seatAngle = values['seat_angle']

        if 'seat_angle_offset' in values and connected:
            self.Angle.angleOffset = values['seat_angle_offset']

    def update_travel_data(self, values: dict):
        # Reset values
        self.Travel.reset()

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

            # Subscribe to topic(s)
            # 🤔 Note that we subscribe *after* starting the message
            # loggers. Otherwise, we may miss retained messages.
            # TODO replace rawdata by individual topics
            # await client.subscribe("sensors/rawData")
            # manager = client.filtered_messages('sensors/rawData')
            # messages = await stack.enter_async_context(manager)
            # task = asyncio.create_task(handle_sensors_rawData(client, messages, chair_state))
            # tasks.add(task)

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
        await asyncio.sleep(1)


async def log_messages(messages, template):
    async for message in messages:
        # 🤔 Note that we assume that the message paylod is an
        # UTF8-encoded string (hence the `bytes.decode` call).
        print(template.format(message.payload.decode()))


async def handle_sensors_rawData(client, messages, chair_state: ChairState):
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
    async for message in messages:
        # print('rawData', message.payload.decode())
        values = json.loads(message.payload.decode())
        chair_state.updateRawData(values)


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
        # print('angle', message.payload.decode())
        values = json.loads(message.payload.decode())
        chair_state.update_travel_data(values)


async def cancel_tasks(tasks):
    for task in tasks:
        if task.done():
            continue
        task.cancel()
        try:
            await task
        except asyncio.CancelledError:
            pass


async def chair_state_main():
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
    asyncio.run(chair_state_main())
