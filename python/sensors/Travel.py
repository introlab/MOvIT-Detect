# from MPU6050 import mpu6050
from lib_movit_sensors.MPU6050_improved import mpu6050_safe as mpu6050
from lib_movit_sensors.IMUDetectMotion import IMUDetectMotion
from datetime import datetime
import asyncio
import aiofiles
from contextlib import AsyncExitStack, asynccontextmanager
from asyncio_mqtt import Client, MqttError
import json
import math

class TravelState(IMUDetectMotion):
    def __init__(self,config):
        super().__init__()
        self.timestamp = int(datetime.now().timestamp())
        self.connected = False
        self.isMoving = False
        self.lastDistance = 0
        self.travelX = 0
        self.travelY = 0
        self.error_count = 0

        if config.has_section('Travel'):
            self.setConfig(config)


    def setConfig(self,config):
        super().setAttributes(sizeSlidingWindow = config.getint('Travel','sizeSlidingWindow'),    
                                thresholdAcc = config.getfloat('Travel','thresholdAcc'),
                                thresholdGyro = config.getfloat('Travel','thresholdGyro')
                                )

    def reset(self):
        super().__init__()
        self.timestamp = int(datetime.now().timestamp())
        self.connected = False
        self.isMoving = False
        self.lastDistance = 0
        self.travelX = 0
        self.travelY = 0
        self.error_count = 0

    def update(self, travel: mpu6050):
        try:

            # For debug
            # self.error_count = travel.get_error_count()

            if travel.connected():
                self.timestamp = int(datetime.now().timestamp())
                self.connected = True
                self.isMoving = False

                # Reset IMU every time ?
                # This way we are sure the device is reinitialized if unplugged/plugged
                # travel.reset()
                travel.wake_up()
                
                self.addIMUData(travel.get_all_data(raw=True))
                
                self.isMoving = bool(self.isMotion())
                
                if self.isMoving:
                    self.lastDistance = 101
                else:
                    self.lastDistance = 0

                # self.travelX, self.travelY = travel.get_motion_slow()
                # self.lastDistance = math.sqrt(self.travelX * self.travelX + self.travelY * self.travelY)
                # if self.lastDistance > 100.0:
                #     self.isMoving = True
            else:
                travel.reset()
                self.reset()
        except RuntimeError as e:
            print(e, 'Attempting reset.')
            try:
                travel.reset()
            except Exception as e:
                pass
            finally:
                self.reset()

    def to_dict(self):
        return {
            'sensorName': 'PMW3901',
            'timestamp': self.timestamp,
            'connected': self.connected,
            'isMoving': self.isMoving,
            'lastDistance': self.lastDistance,
            'travelX': self.travelX,
            'travelY': self.travelY,
            'error_count': self.error_count
        }

    def from_dict(self, values: dict):
        if 'timestamp' in values:
            self.timestamp = values['timestamp']

        if 'connected' in values:
            self.connected = values['connected']
        
        if 'isMoving' in values:
            self.isMoving = values['isMoving']

        if 'lastDistance' in values:
            self.lastDistance = values['lastDistance']

        if 'travelX' in values:
            self.travelX = values['travelX']

        if 'travelY' in values:
            self.travelY = values['travelY']

        if 'error_count' in values:
            self.error_count = values['error_count']

    def to_json(self):
        return json.dumps(self.to_dict())

    def from_json(self, data: str):
        values = json.loads(data)
        self.from_dict(values)

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

            # Create PWM3901 driver
            # travel = PMW3901()
            # Create mpu6050 driver
            travel = mpu6050(address=0x68)

            # Create Travel State
            state = TravelState(config)

            # Messages that doesn't match a filter will get logged here
            messages = await stack.enter_async_context(client.unfiltered_messages())
            task = asyncio.create_task(log_messages(messages, "[unfiltered] {}"))
            tasks.add(task)


            # Start periodic publish of travel state
            task = asyncio.create_task(travel_loop(client, travel, state,config))
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

async def travel_loop(client, travel: mpu6050, state: TravelState, config):
    last_publish = datetime.now()
    while True:
        state.update(travel)

        if int(float(datetime.now().timestamp()-last_publish.timestamp())) >= config.getfloat('Travel','publishPeriod'):
            # Publish state
            await client.publish('sensors/travel', state.to_json())
            last_publish = datetime.now()

        await asyncio.sleep(config.getfloat('Travel','samplingPeriod'))


async def travel_main(config):
    reconnect_interval = 3  # [seconds]

    while True:
        try:
            await connect_to_mqtt_server(config)
        except MqttError as error:
            print(f'MqttError "{error}". Reconnecting in {reconnect_interval} seconds.')
        except RuntimeError as error:
            print(f'RuntimeError "{error}". Reconnecting in {reconnect_interval} seconds.')
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
    argument_parser = argparse.ArgumentParser(description='Travel')
    argument_parser.add_argument('--config', type=str, help='Specify config file', default='../config.cfg')
    args = argument_parser.parse_args()


    config_parser = configparser.ConfigParser()

    print("Opening configuration file : ", args.config)
    read_ok = config_parser.read(args.config)

    if not len(read_ok):
        print('Cannot load config file', args.config)
        exit(-1)

    # main task
    asyncio.run(travel_main(config_parser))