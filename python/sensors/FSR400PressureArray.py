from  lib_movit_sensors.PressureMat import PressureMat
from  lib_movit_sensors.MAX11611 import MAX11611
import numpy as np
import time
import paho.mqtt.client as mqtt
import json
import argparse
import configparser

class FSR400PressureArray(PressureMat):
    def __init__(self,config):
        # ADC instance, default parameters
        # Initialized before PressureMat constructor to make sure adc exists
        self.adc = MAX11611()
        self.array_size = 9

        PressureMat.__init__(self, self.array_size)

    def init_coordinates(self):
        """
        {'x': 4.0, 'y': 4.0},
        {'x': 4.0, 'y': 0.0},
        {'x': 4.0, 'y': -4.0},
        {'x': 0.0, 'y': 4.0},
        {'x': 0.0, 'y': 0.0},
        {'x': 0.0, 'y': -4.0},
        {'x': -4.0, 'y': 4.0},
        {'x': -4.0, 'y': 0.0},
        {'x': -4.0, 'y': -4.0}
        """
        self.coordinates[0] = [4.0, 4.0]
        self.coordinates[1] = [4.0, 0.0]
        self.coordinates[2] = [4.0, -4.0]
        self.coordinates[3] = [0.0, 4.0]
        self.coordinates[4] = [0.0, 0.0]
        self.coordinates[5] = [0.0, -4.0]
        self.coordinates[6] = [-4.0, 4.0]
        self.coordinates[7] = [-4.0, 0.0]
        self.coordinates[8] = [-4.0, -4.0]

    def connected(self):
        return self.adc.connected()

    def init_values(self):
        self.values = self.adc.get_values()
    
    def init_offsets(self):
        self.offsets = np.zeros(self.array_size)

    def update(self):
        # update timestamp
        super().update()
        
        # Read sensor
        if self.connected():
            # Always reconfigure device
            self.adc.config()
            # Read data
            self.values = self.adc.read_adc()
        else:
            self.values = np.zeros(self.array_size)


if __name__ == "__main__":
    
    # Make sure current path is this file path
    import os
    abspath = os.path.abspath(__file__)
    dname = os.path.dirname(abspath)
    os.chdir(dname)

    # Look for arguments
    argument_parser = argparse.ArgumentParser(description='FSR400PressureArray')
    argument_parser.add_argument('--config', type=str, help='Specify config file', default='../config.cfg')
    args = argument_parser.parse_args()


    config_parser = configparser.ConfigParser()

    print("Opening configuration file : ", args.config)
    read_ok = config_parser.read(args.config)

    if not len(read_ok):
        print('Cannot load config file', args.config)
        exit(-1)

    mat = FSR400PressureArray(config_parser)

    # Create MQTT client
    client = mqtt.Client(None)
    client.username_pw_set(username=config_parser.get('MQTT','usr'), password=config_parser.get('MQTT','pswd'))
    client.connect(host=config_parser.get('MQTT','broker_address'), port=config_parser.getint('MQTT','broker_port'))
    
    while True:    
        # Always update
        mat.update()
        client.publish('sensors/pressure', mat.to_json())
        # Wait for next cycle
        time.sleep(1)
