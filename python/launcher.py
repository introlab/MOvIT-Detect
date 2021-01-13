
import os
import subprocess
import threading
import sys
import pathlib
import time
import argparse


def launch_process(script: str, working_directory: str, *args):
    executable_args = [sys.executable, script] + [*args]

    # stdout=os.subprocess.PIPE, stderr=os.subprocess.PIPE)
    try:
        process = subprocess.Popen(executable_args,
                            cwd=os.path.realpath(working_directory))


        return process
    except OSError as e:
        print(' - error starting process:', e)

    return None


if __name__ == "__main__":
    # We will launch sensor process
     # Look for arguments
    argument_parser = argparse.ArgumentParser(description='Launcher')
    argument_parser.add_argument('--config', type=str, help='Specify config file', default=os.path.dirname(os.path.abspath(__file__)) + '/config.cfg')
    args = argument_parser.parse_args()


    # TODO Change to fit your needs
    # TODO Restart process if crash

    process_list = []

    # Global configuration file
    print('Launcher config file:', args.config)
    process_args = ['--config=' + args.config]


    # Sensors process
    # Working directory is current file path + sensors_path
    sensors_path = '/sensors'
    process_list.append(launch_process('Alarm.py', os.path.dirname(os.path.abspath(__file__))  + sensors_path, *process_args))
    process_list.append(launch_process('Travel.py', os.path.dirname(os.path.abspath(__file__)) + sensors_path, *process_args))
    process_list.append(launch_process('IMUSeatAngle.py', os.path.dirname(os.path.abspath(__file__)) + sensors_path, *process_args))
    process_list.append(launch_process('FSR400PressureArray.py', os.path.dirname(os.path.abspath(__file__)) + sensors_path, *process_args))


    # FSM Process (all running in the same process for now, can be separated as needed)
    fsm_path = '/fsm'
    process_list.append(launch_process('fsm_main.py', os.path.dirname(os.path.abspath(__file__))  + fsm_path, *process_args))

    while True:
        for process in process_list:
            # print(process)
            pass
        # Wait for 1 sec
        # Wait for interrupt, publish stats?
        time.sleep(1)