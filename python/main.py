import asyncio
import aiofiles
from contextlib import AsyncExitStack, asynccontextmanager
from random import randrange
from asyncio_mqtt import Client, MqttError
from ChairState import ChairState, TravelInformation, AngleInformation, PressureInformation
import json


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
            for topic_filter in topic_filters:
                # Log all messages that matches the filter
                manager = client.filtered_messages(topic_filter)
                messages = await stack.enter_async_context(manager)
                template = f'[topic_filter="{topic_filter}"] {{}}'
                task = asyncio.create_task(log_messages(messages, template))
                tasks.add(task)

            # Messages that doesn't match a filter will get logged here
            messages = await stack.enter_async_context(client.unfiltered_messages())
            task = asyncio.create_task(log_messages(messages, "[unfiltered] {}"))
            tasks.add(task)

            # Subscribe to topic(s)
            # 🤔 Note that we subscribe *after* starting the message
            # loggers. Otherwise, we may miss retained messages.
            await client.subscribe("sensors/rawData")
            manager = client.filtered_messages('sensors/rawData')
            messages = await stack.enter_async_context(manager)
            task = asyncio.create_task(handle_sensors_rawData(client, messages, chair_state))
            tasks.add(task)

            # Start periodic publish of chair state
            task = asyncio.create_task(publish_chair_state(client, chair_state))
            tasks.add(task)

            # Wait for everything to complete (or fail due to, e.g., network errors)
            await asyncio.gather(*tasks)


async def publish_chair_state(client, chair_state):
    # 1Hz
    while True:
        print('publish chair state', chair_state)
        await client.publish('sensors/chairState', chair_state.to_json(), qos=2)
        await asyncio.sleep(1)


async def log_messages(messages, template):
    async for message in messages:
        # 🤔 Note that we assume that the message paylod is an
        # UTF8-encoded string (hence the `bytes.decode` call).
        print(template.format(message.payload.decode()))


async def handle_sensors_rawData(client, messages, chair_state):
    async for message in messages:
        print('rawData', message.payload.decode())


async def cancel_tasks(tasks):
    for task in tasks:
        if task.done():
            continue
        task.cancel()
        try:
            await task
        except asyncio.CancelledError:
            pass


async def main():
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

# main task
asyncio.run(main())
