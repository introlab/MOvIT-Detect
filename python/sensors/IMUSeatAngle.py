from SeatAngle import SeatAngle
from MPU6050 import mpu6050
import numpy as np
import time
import paho.mqtt.client as mqtt
import json
import argparse
import configparser
import os
import math
from lib_movit_sensors.AngleAnalysis import AngleAnalysis
from enum import Enum, unique

@unique
class IMUSeatAngleState(Enum):
    INIT = 0
    IMU_ERROR = 1
    CHECK_CALIBRATION_VALID = 2
    CALIBRATION_WAIT_ZERO_TRIG = 3
    CALIBRATION_RUNNING_ZERO = 4
    CALIBRATION_WAIT_INCLINED_TRIG = 5
    CALIBRATION_RUNNING_INCLINED = 6
    CALIBRATION_WAIT_ROT_WORLD_CALC = 7
    CALIBRATION_DONE = 8
    RUNNING_CALIBRATED = 9

class IMUSeatAngle(SeatAngle):

    def __init__(self):
        SeatAngle.__init__(self)
        self.fixed_imu = mpu6050(address=0x68)
        self.mobile_imu = mpu6050(address=0x69)
        self.fixed_imu_data = self.fixed_imu.get_all_data(raw=True)
        self.mobile_imu_data = self.mobile_imu.get_all_data(raw=True)
        self.aa = AngleAnalysis()
        self.state = IMUSeatAngleState.INIT


    def connected(self) -> bool:
        return self.fixed_imu.connected() and self.mobile_imu.connected()

    def initialize_angle_analysis(self, client, config):
        self.aa.intializeIMU(client, config)


    def calibrated(self):
        return self.state == IMUSeatAngleState.RUNNING_CALIBRATED

    def calib_trigger(self):
        if self.state == IMUSeatAngleState.CALIBRATION_WAIT_ZERO_TRIG:
            self.state = IMUSeatAngleState.CALIBRATION_RUNNING_ZERO
        elif self.state == IMUSeatAngleState.CALIBRATION_WAIT_INCLINED_TRIG:
            self.state = IMUSeatAngleState.CALIBRATION_RUNNING_INCLINED
        else:
            # Trigger when in another state will start a new calibration
            self.state = IMUSeatAngleState.CALIBRATION_RUNNING_ZERO

    def to_dict(self) -> dict:
        result = super().to_dict()   

        # Add IMU DATA
        result['fixed_imu'] = {'connected': self.fixed_imu.connected(), 
                            'accelerometers': self.fixed_imu_data[0], 
                            'gyros': self.fixed_imu_data[1], 
                            'temperature': self.fixed_imu_data[2]}

        result['mobile_imu'] = {'connected': self.mobile_imu.connected(), 
                            'accelerometers': self.mobile_imu_data[0], 
                            'gyros': self.mobile_imu_data[1], 
                            'temperature': self.mobile_imu_data[2]}

        result['state'] = {'stateNumber': self.state.value, 
                        'stateName': self.state.name}

        return result

    def calculate_angle(self) -> float :
        # Simple way to calculate angles
        # TODO for new algorithm 
        fixed_angle = self.calculate_IMU_angle(self.fixed_imu_data[0]['x'], self.fixed_imu_data[0]['y'], self.fixed_imu_data[0]['z'])
        mobile_angle = self.calculate_IMU_angle(self.mobile_imu_data[0]['x'], self.mobile_imu_data[0]['y'], self.mobile_imu_data[0]['z'])
        return - (fixed_angle - mobile_angle - self.seat_angle_offset)


    def calculate_IMU_angle(self, acc_x, acc_y, acc_z) -> float:
        # Old implementation to be replaced by something better
        y = -acc_x
        x = math.sqrt(acc_y * acc_y + acc_z * acc_z)
        # Convert radians to degrees
        return math.atan2(y, x) * (180.0 / math.pi)

    def update(self, client, config) -> float:
        # will update timestamp
        super().update()

        # Reset IMU every time ?
        # This way we are sure the device is reinitialized if unplugged/plugged
        self.fixed_imu.reset()
        self.mobile_imu.reset()

        self.fixed_imu_data = self.fixed_imu.get_all_data(raw=True)
        self.mobile_imu_data = self.mobile_imu.get_all_data(raw=True)

        # default value, not ready!
        self.seat_angle = 0

        client.publish(config.get('MQTT', 'topic_publish'), self.to_json())

        # State machine here
        if not self.connected():
            self.state = IMUSeatAngleState.IMU_ERROR
        else:    
            # We are connected...
            if self.state == IMUSeatAngleState.INIT:
                # Next step is to verify calib
                self.state = IMUSeatAngleState.CHECK_CALIBRATION_VALID
                
            elif self.state == IMUSeatAngleState.IMU_ERROR:
                # Was in error (disconnected), go back to INIT state
                self.state = IMUSeatAngleState.INIT
                
            elif self.state == IMUSeatAngleState.CHECK_CALIBRATION_VALID:
                # TODO read file, if not valid wait for calib trigger
                if self.aa.isRotWorld:
                    self.state = IMUSeatAngleState.CALIBRATION_DONE
                else:
                    self.initialize_angle_analysis(client,config)
                    # If calibration not ready, wait for trigger...
                    self.state = IMUSeatAngleState.CALIBRATION_WAIT_ZERO_TRIG
            
            elif self.state == IMUSeatAngleState.CALIBRATION_WAIT_ZERO_TRIG:
                # Wait trigger, must receive a MQTT signal on 'config/calib_imu'
                pass
            elif self.state == IMUSeatAngleState.CALIBRATION_RUNNING_ZERO:
                # WAIT FOR ENOUGH DATA TO ZERO CALIBRATION
                self.aa.isAtZero = True
                self.aa.isInclined = False
                if not self.aa.askAtZero:
                    self.state = IMUSeatAngleState.CALIBRATION_WAIT_INCLINED_TRIG

            elif self.state == IMUSeatAngleState.CALIBRATION_WAIT_INCLINED_TRIG:
                # Wait trigger, must receive a MQTT signal on 'config/calib_imu'
                pass
            elif self.state == IMUSeatAngleState.CALIBRATION_RUNNING_INCLINED:
                # WAIT FOR ENOUGH DATA TO INCLINED CALIBRATION
                self.aa.isAtZero = False
                self.aa.isInclined = True
                if not self.aa.askInclined:
                    self.state = IMUSeatAngleState.CALIBRATION_WAIT_ROT_WORLD_CALC

            elif self.state == IMUSeatAngleState.CALIBRATION_WAIT_ROT_WORLD_CALC:
                # WAIT FOR CALCULATION DONE
                if self.aa.isRotWorld:
                    # GO TO CALIBRATION DONE 
                    self.state = IMUSeatAngleState.CALIBRATION_DONE

            elif self.state == IMUSeatAngleState.CALIBRATION_DONE:
                # SAVE CALIBRATION FILE
                self.aa.isAtZero = False
                self.aa.isInclined = False

                # self.aa.saveToJson()
                self.aa.startGetAngle(client, config)

                # GO TO RUNNING MODE
                self.state = IMUSeatAngleState.RUNNING_CALIBRATED

            elif self.state == IMUSeatAngleState.RUNNING_CALIBRATED:
                # SEAT CALCULATION
                if self.aa.isRotWorld:
                    result = self.aa.getAngleAnalysis()
                    self.seat_angle = result['angleSiege']

                # FOR NOW OLD CALCULATION
                # self.seat_angle = self.calculate_angle()

        print('State: ', self.state.name)
        return self.seat_angle


def createClient(clientName,config):
    #create client connected to mqtt broker
    
    #create new instance
    print("MQTT creating new instance named : "+clientName)
    client = mqtt.Client(clientName) 
    
    #set username and password
    print("MQTT setting  password")
    client.username_pw_set(username=config.get('MQTT','usr'),password=config.get('MQTT','pswd'))
    
    #connection to broker
    broker_address=config.get('MQTT','broker_address')
    print("MQTT connecting to broker : "+broker_address)
    client.connect(broker_address) #connect to broker
    
    return client


if __name__ == "__main__":
    
    # Make sure current path is this file path
    import os

    abspath = os.path.abspath(__file__)
    dname = os.path.dirname(abspath)
    os.chdir(dname)

    # Look for arguments
    argument_parser = argparse.ArgumentParser(description='IMUSeatAngle')
    argument_parser.add_argument('--config', type=str, help='Specify config file', default='../config.cfg')
    args = argument_parser.parse_args()


    ############
    # import config file
    config = configparser.ConfigParser()

    print('opening configuration file :', args.config)
    ret = config.read(args.config)

    ############
    # connect to mqtt broker
    client = createClient("LoggerAngle", config)

    # Will start background thread for async messages read/write
    client.loop_start()

    imu = IMUSeatAngle()

    # This is forcing calibration.
    # imu.initialize_angle_analysis(client,config)

    def on_message(client, userdata: IMUSeatAngle, message):
        print(client, userdata, message)
        if message.topic == 'config/calib_imu':
            imu.calib_trigger()

    # Set userdata
    client.user_data_set(imu)

    # Set callback
    client.on_message = on_message

    # Subscribe to calibration topics
    client.subscribe('config/calib_imu')
    

    # Main loop
    while True:

        # Update angle (depending on state...)
        imu.seat_angle = imu.update(client, config)

        # Publish real angle value
        client.publish('sensors/angle', imu.to_json())

        time.sleep(1)
