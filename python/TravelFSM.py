from datetime import datetime
import asyncio
import aiofiles
from contextlib import AsyncExitStack, asynccontextmanager
from asyncio_mqtt import Client, MqttError
from ChairState import ChairState, TravelInformation, AngleInformation, PressureInformation
import json


# TravelFSM: (fsm/travel)
# {
#   "time": 1604434147,
#   "elapsed": 0,
#   "event": "Other",
#   "stateNum": 0,
#   "stateName": "INIT"
# }

class TravelFSMState:
    def __init__(self):
        self.type = 'TravelFSMState'
        self.timestamp = int(datetime.now().timestamp())
        self.elapsed = 0
        self.event = "Other"
        self.stateNum = 0
        self.stateName = "INIT"

    def to_dict(self):
        return {
            'type': self.type,
            'time': self.timestamp,
            'elapsed': self.elapsed,
            'event': self.event,
            'stateNum': self.stateNum,
            'stateName': self.stateName
        }

    def from_dict(self, values):
        if 'type' in values and values['type'] == self.type:
            if 'time' in values:
                self.timestamp = values['time']

            if 'elapsed' in values:
                self.elapsed = values['elapsed']

            if 'event' in values:
                self.event = values['event']

            if 'stateNum' in values:
                self.stateNum = values['event']

            if 'stateName' in values:
                self.stateName = values['stateName']
            return True
        return False

    def reset(self):
        self.timestamp = int(datetime.now().timestamp())
        self.elapsed = 0
        self.event = "Other"
        self.stateNum = 0
        self.stateName = "INIT"

    def to_json(self):
        return json.dumps(self.to_dict())

    def from_json(self, data: str):
        values = json.loads(data)
        self.from_dict(values)


class TravelFSM:
    def __init__(self):
        self.state = TravelFSMState()
        self.chairState = ChairState()

    def setChairState(self, state: ChairState):
        self.chairState = state

    def update(self):
        pass

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
            fsm = TravelFSM()

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
        state = ChairState()
        state.from_json(message.payload.decode())
        fsm.setChairState(state)


async def travel_fsm_main():
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
    asyncio.run(travel_fsm_main())

