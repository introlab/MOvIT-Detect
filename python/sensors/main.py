
import os
import subprocess
import threading
import sys
import pathlib
import time

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
    # TODO Handle args
    # TODO Change to fit your needs
    # TODO Restart process if crash

    process_list = []

    # Working directory is current file path
    process_list.append(launch_process('Alarm.py', os.path.dirname(os.path.abspath(__file__))))
    process_list.append(launch_process('Travel.py', os.path.dirname(os.path.abspath(__file__))))
    process_list.append(launch_process('IMUSeatAngle.py', os.path.dirname(os.path.abspath(__file__))))
    process_list.append(launch_process('FSR400PressureArray.py', os.path.dirname(os.path.abspath(__file__))))

    while True:
        for process in process_list:
            # print(process)
            pass
        
        # Wait for 1 sec
        time.sleep(1)