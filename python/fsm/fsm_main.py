import asyncio
import aiofiles
from contextlib import AsyncExitStack, asynccontextmanager
import json


async def movit_main_original(config):
    from ChairState import chair_state_main
    from AngleFSM import angle_fsm_main
    from TravelFSM import travel_fsm_main
    from SeatingFSM import seating_fsm_main
    from NotificationFSM import notification_fsm_main
    async with AsyncExitStack() as stack:
        # Keep track of the asyncio tasks that we create, so that
        # we can cancel them on exit
        tasks = set()
        stack.push_async_callback(cancel_tasks, tasks)

        # Start all tasks

        # Chair State
        task = asyncio.create_task(chair_state_main(config))
        tasks.add(task)

        # AngleFSM
        task = asyncio.create_task(angle_fsm_main(config))
        tasks.add(task)

        # TravelFSM
        task = asyncio.create_task(travel_fsm_main(config))
        tasks.add(task)

        # SeatingFSM
        task = asyncio.create_task(seating_fsm_main(config))
        tasks.add(task)

        # NotificationFSM
        task = asyncio.create_task(notification_fsm_main(config))
        tasks.add(task)

        # Wait for everything to complete (or fail due to, e.g., network errors)
        await asyncio.gather(*tasks)


async def cancel_tasks(tasks):
    for task in tasks:
        if task.done():
            continue
        task.cancel()
        try:
            await task
        except asyncio.CancelledError:
            pass

if __name__ == "__main__":

    # Make sure current path is this file path
    import os
    import argparse
    import configparser
    abspath = os.path.abspath(__file__)
    dname = os.path.dirname(abspath)
    os.chdir(dname)

    # Look for arguments
    argument_parser = argparse.ArgumentParser(description='main (FSM)')
    argument_parser.add_argument('--config', type=str, help='Specify config file', default='../config.cfg')
    args = argument_parser.parse_args()


    config_parser = configparser.ConfigParser()

    print("Opening configuration file : ", args.config)
    read_ok = config_parser.read(args.config)

    if not len(read_ok):
        print('Cannot load config file', args.config)
        exit(-1)

    # main task will start all others
    asyncio.run(movit_main_original(config_parser))
