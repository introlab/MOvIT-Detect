from PMW3901 import PMW3901
from datetime import datetime
import asyncio
import aiofiles
from contextlib import AsyncExitStack, asynccontextmanager
from asyncio_mqtt import Client, MqttError
import json
import math

class TravelState:
    def __init__(self):
        self.timestamp = int(datetime.now().timestamp())
        self.connected = False
        self.isMoving = False
        self.lastDistance = 0
        self.error_count = 0

    def reset(self):
        self.timestamp = int(datetime.now().timestamp())
        self.connected = False
        self.isMoving = False
        self.lastDistance = 0
        self.travelX = 0
        self.travelY = 0
        self.error_count = 0

    def update(self, travel: PMW3901):
        try:

            # For debug
            self.error_count = travel.get_error_count()

            if travel.connected():
                self.timestamp = int(datetime.now().timestamp())
                self.connected = True
                self.isMoving = False
                self.travelX, self.travelY = travel.get_motion_slow()
                self.lastDistance = math.sqrt(self.travelX * self.travelX + self.travelY * self.travelY)
                if self.lastDistance > 100.0:
                    self.isMoving = True
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

        if 'server' in config:
            # Connect to the MQTT broker
            # client = Client("10.0.1.20", username="admin", password="movitplus")
            client = Client(config['server']['hostname'],
                            username=config['server']['username'],
                            password=config['server']['password'])

            await stack.enter_async_context(client)

            # Create PWM3901 driver
            travel = PMW3901()

            # Create Travel State
            state = TravelState()

            # Messages that doesn't match a filter will get logged here
            messages = await stack.enter_async_context(client.unfiltered_messages())
            task = asyncio.create_task(log_messages(messages, "[unfiltered] {}"))
            tasks.add(task)


            # Start periodic publish of travel state
            task = asyncio.create_task(travel_loop(client, travel, state))
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

async def travel_loop(client, travel: PMW3901, state: TravelState):
    while True:
        state.update(travel)
        # Publish state
        print('publishing', state.to_json())
        await client.publish('sensors/travel', state.to_json())

        # Wait next cycle, 1Hz
        await asyncio.sleep(1)

async def travel_main():
    reconnect_interval = 3  # [seconds]

    # Read config file
    async with aiofiles.open('sensors/config.json', mode='r') as f:
        data = await f.read()
        config = json.loads(data)

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
    # main task
    asyncio.run(travel_main())