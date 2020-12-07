from PCA9536 import pca9536
from datetime import datetime
import asyncio
import aiofiles
from contextlib import AsyncExitStack, asynccontextmanager
from asyncio_mqtt import Client, MqttError
import json


class AlarmState:
    def __init__(self):
        self.enabled = False
        self.isGreenAlarmOn = False
        self.isRedAlarmOn = False
        self.isBlinkRedAlarmOn = False
        self.isBlinkLedsAlarmOn = False
        self.isBlinkGreenAlarmOn = False
        self.isAlternatingAlarmOn = False
        self.isMotorAlarmOn = False
        self.isButtonOn = False

    def reset(self):
        self.enabled = False
        self.isGreenAlarmOn = False
        self.isRedAlarmOn = False
        self.isBlinkRedAlarmOn = False
        self.isBlinkLedsAlarmOn = False
        self.isBlinkGreenAlarmOn = False
        self.isAlternatingAlarmOn = False
        self.isMotorAlarmOn = False
        self.isButtonOn = False

    def to_dict(self):
        return {
            'enabled': self.enabled,
            'isGreenAlarmOn': self.isGreenAlarmOn,
            'isRedAlarmOn': self.isRedAlarmOn,
            'isBlinkRedAlarmOn': self.isBlinkRedAlarmOn,
            'isBlinkGreenAlarmOn': self.isBlinkGreenAlarmOn,
            'isBlinkLedsAlarmOn': self.isBlinkLedsAlarmOn,
            'isAlternatingAlarmOn': self.isAlternatingAlarmOn,
            'isMotorAlarmOn': self.isMotorAlarmOn,
            'isButtonOn': self.isButtonOn
        }

    def to_json(self):
        return json.dumps(self.to_dict())


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

            # Create PCA9536 driver
            pca = pca9536()

            # Create Alarm State
            state = AlarmState()

            # Messages that doesn't match a filter will get logged here
            messages = await stack.enter_async_context(client.unfiltered_messages())
            task = asyncio.create_task(log_messages(messages, "[unfiltered] {}"))
            tasks.add(task)

            # sensors/alarm/enabled
            await client.subscribe("sensors/alarm/enabled")
            manager = client.filtered_messages('sensors/alarm/enabled')
            messages = await stack.enter_async_context(manager)
            task = asyncio.create_task(handle_alarm_enabled(client, messages, pca, state))
            tasks.add(task)

            # sensors/alarm/alternatingLedBlink
            await client.subscribe("sensors/alarm/alternatingLedBlink")
            manager = client.filtered_messages('sensors/alarm/alternatingLedBlink')
            messages = await stack.enter_async_context(manager)
            task = asyncio.create_task(handle_alarm_alternating_led_blink(client, messages, pca, state))
            tasks.add(task)

            # sensors/alarm/motorOn
            await client.subscribe("sensors/alarm/motorOn")
            manager = client.filtered_messages('sensors/alarm/motorOn')
            messages = await stack.enter_async_context(manager)
            task = asyncio.create_task(handle_alarm_motor_on(client, messages, pca, state))
            tasks.add(task)

            # sensors/alarm/redLedOn
            await client.subscribe("sensors/alarm/redLedOn")
            manager = client.filtered_messages('sensors/alarm/redLedOn')
            messages = await stack.enter_async_context(manager)
            task = asyncio.create_task(handle_alarm_red_led_on(client, messages, pca, state))
            tasks.add(task)

            # sensors/alarm/redLedBlink
            await client.subscribe("sensors/alarm/redLedBlink")
            manager = client.filtered_messages('sensors/alarm/redLedBlink')
            messages = await stack.enter_async_context(manager)
            task = asyncio.create_task(handle_alarm_red_led_blink(client, messages, pca, state))
            tasks.add(task)

            # sensors/alarm/greenLedOn
            await client.subscribe("sensors/alarm/greenLedOn")
            manager = client.filtered_messages('sensors/alarm/greenLedOn')
            messages = await stack.enter_async_context(manager)
            task = asyncio.create_task(handle_alarm_green_led_on(client, messages, pca, state))
            tasks.add(task)

            # sensors/alarm/greenLedBlink
            await client.subscribe("sensors/alarm/greenLedBlink")
            manager = client.filtered_messages('sensors/alarm/greenLedBlink')
            messages = await stack.enter_async_context(manager)
            task = asyncio.create_task(handle_alarm_green_led_blink(client, messages, pca, state))
            tasks.add(task)

            # Start periodic publish of chair state
            task = asyncio.create_task(alarm_loop(client, pca, state))
            tasks.add(task)

        # Wait for everything to complete (or fail due to, e.g., network errors)
        await asyncio.gather(*tasks)

async def handle_alarm_enabled(client, messages, pca: pca9536, state: AlarmState):
    # Topic: sensors/alarm/enabled
    async for message in messages:
        try:
            enabled = int(message.payload.decode())
            # print('handle_alarm_enabled', enabled)
            # Update State
            state.enabled = enabled
            if not enabled:
                state.reset()
        except Exception as e:
            print(e)

async def handle_alarm_alternating_led_blink(client, messages, pca: pca9536, state: AlarmState):
    # Topic: sensors/alarm/alternatingLedBlink
    async for message in messages:
        try:
            enabled = int(message.payload.decode())
            # print('handle_alarm_alternating_led_blink', enabled)
            # Update state
            state.isGreenAlarmOn = False
            state.isRedAlarmOn = False
            state.isBlinkRedAlarmOn = False
            state.isBlinkLedsAlarmOn = False
            state.isBlinkGreenAlarmOn = False
            state.isAlternatingAlarmOn = enabled
        except Exception as e:
            print(e)

async def handle_alarm_motor_on(client, messages, pca: pca9536, state: AlarmState):
    # Topic: sensors/alarm/motorOn
    async for message in messages:
        try:
            enabled = int(message.payload.decode())
            # print('handle_alarm_motor_on', enabled)
            # Update state
            state.isMotorAlarmOn = enabled
        except Exception as e:
            print(e)

async def handle_alarm_red_led_on(client, messages, pca: pca9536, state: AlarmState):
    # Topic: sensors/alarm/redLedOn
    async for message in messages:
        try:
            enabled = int(message.payload.decode())
            # print('handle_alarm_red_led_on', enabled)
            # Update state
            state.isRedAlarmOn = enabled
            state.isBlinkRedAlarmOn = False
            state.isBlinkLedsAlarmOn = False
            state.isAlternatingAlarmOn = False
        except Exception as e:
            print(e)


async def handle_alarm_red_led_blink(client, messages, pca: pca9536, state: AlarmState):
    # Topic: sensors/alarm/redLedBlink
    async for message in messages:
        try:
            enabled = int(message.payload.decode())
            # print('handle_alarm_red_led_blink', enabled)
            # Update state
            state.isRedAlarmOn = False
            state.isBlinkRedAlarmOn = True
            state.isBlinkLedsAlarmOn = False
            state.isAlternatingAlarmOn = False
        except Exception as e:
            print(e)

async def handle_alarm_green_led_on(client, messages, pca: pca9536, state: AlarmState):
    # Topic: sensors/alarm/greenLedOn
    async for message in messages:
        try:
            enabled = int(message.payload.decode())
            # print('handle_alarm_green_led_on', enabled)
            # Update state
            state.isGreenAlarmOn = enabled
            state.isBlinkLedsAlarmOn = False
            state.isBlinkGreenAlarmOn = False
            state.isAlternatingAlarmOn = False    
        except Exception as e:
            print(e)

async def handle_alarm_green_led_blink(client, messages, pca: pca9536, state: AlarmState):
    # Topic: sensors/alarm/greenLedBlink
    async for message in messages:
        try:
            enabled = int(message.payload.decode())
            # print('handle_alarm_green_led_blink', enabled)
            # Update state
            state.isGreenAlarmOn = False
            state.isBlinkLedsAlarmOn = False
            state.isBlinkGreenAlarmOn = enabled
            state.isAlternatingAlarmOn = False
        except Exception as e:
            print(e)


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


async def alarm_loop(client, pca: pca9536, state: AlarmState):
    # 10 Hz
    while True:
        if pca.connected():
            if state.enabled:
                # Motor
                if state.isMotorAlarmOn:
                    pca.set_motor(1)
                else:
                    pca.set_motor(0)

                # Red LED
                if state.isRedAlarmOn:
                    pca.set_red_led(1)
                elif state.isBlinkRedAlarmOn: 
                    pca.toggle_red_led()
                elif state.isAlternatingAlarmOn:
                    pca.toggle_red_led()
                else:
                    pca.set_red_led(0)

                # Green LED
                if state.isGreenAlarmOn:
                    pca.set_green_led(1)
                elif state.isBlinkGreenAlarmOn: 
                    pca.toggle_green_led()
                elif state.isAlternatingAlarmOn:
                    # Green led = inverse than red led
                    pca.set_green_led(not pca.get_red_led())
                else:
                    pca.set_green_led(0)
        
            else:
                # Reset
                pca.set_green_led(0)
                pca.set_red_led(0)
                pca.set_motor(0)

            # Read button
            state.isButtonOn = pca.get_button()
        else:
            state.reset()
            state.isButtonOn = pca.get_button()
            pca.reset()

        # Publish state
        await client.publish('sensors/alarm/state', state.to_json())

        # Wait next cycle
        await asyncio.sleep(0.1)

async def alarm_main():
    reconnect_interval = 3  # [seconds]

    # Read config file
    async with aiofiles.open('sensors/config.json', mode='r') as f:
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
    asyncio.run(alarm_main())


