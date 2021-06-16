from lib_movit_sensors.SeatAngle import SeatAngle
# from MPU6050 import mpu6050
from lib_movit_sensors.MPU6050_improved import mpu6050_safe as mpu6050
import numpy as np
from datetime import datetime
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
    AA_ERROR = 2
    CHECK_CALIBRATION_VALID = 3
    CALIBRATION_DONE = 4
    RUNNING_CALIBRATED = 5
    WAITING_FOR_CALIBRATION = 6
    WAITING_FOR_CALIBRATION_TRIGGER = 7
    CALIBRATION_TODO = 8

class IMUSeatAngle(SeatAngle):

    def __init__(self):
        SeatAngle.__init__(self)
        self.fixed_imu = mpu6050(address=0x68)
        self.mobile_imu = mpu6050(address=0x69)
        self.fixed_imu_data = self.fixed_imu.get_all_data(raw=True)
        self.mobile_imu_data = self.mobile_imu.get_all_data(raw=True)
        self.aa = AngleAnalysis()
        self.state = IMUSeatAngleState.INIT
        self.calib_flag = False


    def connected(self) -> bool:
        return self.fixed_imu.connected() and self.mobile_imu.connected()

    def initialize_angle_analysis(self, config):
        self.aa.intializeIMU(config)

    def calibrated(self):
        return self.state == IMUSeatAngleState.RUNNING_CALIBRATED

    def calib_trigger(self, stateBool):
        if (stateBool):
            if IMUSeatAngleState.CALIBRATION_DONE or IMUSeatAngleState.RUNNING_CALIBRATED or IMUSeatAngleState.CALIBRATION_TODO:
                self.state = IMUSeatAngleState.WAITING_FOR_CALIBRATION_TRIGGER
            self.calib_flag = True 
        else:
            self.state = IMUSeatAngleState.INIT

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

        try:
            result['state']['stateNumber AA'] = self.aa.getStateValue()
            result['state']['stateName AA'] = self.aa.getStateName()
        except:
            pass

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

    def update(self, config) -> float:
        # will update timestamp
        super().update()

        # # Reset IMU every time ?
        # # This way we are sure the device is reinitialized if unplugged/plugged
        # self.fixed_imu.reset()
        # self.mobile_imu.reset()
        self.fixed_imu.wake_up()
        self.mobile_imu.wake_up()

        self.fixed_imu_data = self.fixed_imu.get_all_data(raw=True)
        self.mobile_imu_data = self.mobile_imu.get_all_data(raw=True)

        # default value, not ready!
        self.seat_angle = 0

        # State machine here
        if not self.connected():
            self.state = IMUSeatAngleState.IMU_ERROR
        else:    
            # We are connected...
            if self.state == IMUSeatAngleState.INIT:
                if not self.aa.getStateName() == 'INIT':
                    self.aa.__init__()
                # Next step is to verify calib
                self.state = IMUSeatAngleState.CHECK_CALIBRATION_VALID
                
            elif self.state == IMUSeatAngleState.IMU_ERROR:
                # Was in error (disconnected), go back to INIT state
                # of problem with aa
                self.__init__()
                # self.state = IMUSeatAngleState.INIT

            elif self.state == IMUSeatAngleState.AA_ERROR:
                # Was in error (disconnected), go back to INIT state
                # of problem with aa
                self.state = IMUSeatAngleState.INIT
                
            elif self.state == IMUSeatAngleState.CHECK_CALIBRATION_VALID:
                # Wait for angle analysis INIT and 
                if (self.aa.getStateName() == 'INIT' 
                or self.aa.getStateName() == 'CHECK_CALIBRATION_VALID'):
                    self.aa.update()
                elif self.aa.getStateName() == 'CALIBRATION_DONE':
                    self.state = IMUSeatAngleState.CALIBRATION_DONE
                elif self.aa.getStateName() == 'CALIBRATION_TODO':
                    # If calibration not ready, wait for trigger...
                    self.state = IMUSeatAngleState.CALIBRATION_TODO
                else:
                    self.state == IMUSeatAngleState.AA_ERROR

            elif self.state == IMUSeatAngleState.CALIBRATION_DONE:
                self.state = IMUSeatAngleState.RUNNING_CALIBRATED
                pass

            elif self.state == IMUSeatAngleState.RUNNING_CALIBRATED:
                # SEAT CALCULATION
                if not self.aa.getStateName() == "CALIBRATION_DONE":
                    self.state = IMUSeatAngleState.AA_ERROR
                else:
                    # Wait for calibration trigger
                    if self.calib_flag:
                        try:
                            self.calib_flag = False
                            self.aa.update(calibTrigger=True,
                                        fixed_imu_data=self.fixed_imu_data,
                                        mobile_imu_data=self.mobile_imu_data,
                                        config=config)
                            if self.aa.getStateName() == "CALIBRATION_WAIT_ZERO_TRIG":
                                self.state = IMUSeatAngleState.WAITING_FOR_CALIBRATION_TRIGGER
                            else:
                                self.state = IMUSeatAngleState.AA_ERROR
                        except:
                            self.state = IMUSeatAngleState.AA_ERROR
                    else:
                        try:
                            self.aa.update(fixed_imu_data=self.fixed_imu_data,
                                        mobile_imu_data=self.mobile_imu_data)
                            result = self.aa.getAngleAnalysis()
                            self.seat_angle = result['angleSiege']
                        except:
                            self.state = IMUSeatAngleState.AA_ERROR
                pass

            elif self.state == IMUSeatAngleState.WAITING_FOR_CALIBRATION:
                if (self.aa.getStateName() == 'CALIBRATION_RUNNING_ZERO'
                    or self.aa.getStateName() == 'CALIBRATION_RUNNING_INCLINED'):
                    try:
                        self.aa.update(fixed_imu_data=self.fixed_imu_data,
                                    mobile_imu_data=self.mobile_imu_data,
                                    config=config)
                    except:
                        self.state = IMUSeatAngleState.AA_ERROR

                elif (self.aa.getStateName() == 'CALIBRATION_WAIT_ZERO_TRIG' 
                    or self.aa.getStateName() == 'CALIBRATION_WAIT_INCLINED_TRIG'):
                    self.state = IMUSeatAngleState.WAITING_FOR_CALIBRATION_TRIGGER

                elif self.aa.getStateName() == 'CALIBRATION_WAIT_ROT_WORLD_CALC':
                    self.aa.update()
                    if self.aa.getStateName() == 'CALIBRATION_DONE':
                        self.state = IMUSeatAngleState.CALIBRATION_DONE
                    else:
                        self.state = IMUSeatAngleState.AA_ERROR
                    pass

                elif (self.aa.getStateName() == 'CALIBRATION_DONE'):
                    self.state = IMUSeatAngleState.CALIBRATION_DONE

                else:
                    self.state = IMUSeatAngleState.AA_ERROR
                pass

            elif self.state == IMUSeatAngleState.WAITING_FOR_CALIBRATION_TRIGGER or self.state == IMUSeatAngleState.CALIBRATION_TODO:
                if (self.aa.getStateName() == 'CALIBRATION_WAIT_ZERO_TRIG' 
                    or self.aa.getStateName() == 'CALIBRATION_WAIT_INCLINED_TRIG'
                    or self.aa.getStateName() == 'CALIBRATION_TODO'):
                    if self.calib_flag:
                        try:
                            self.calib_flag = False
                            self.aa.update(calibTrigger=True,
                                    fixed_imu_data=self.fixed_imu_data,
                                    mobile_imu_data=self.mobile_imu_data,
                                    config=config)
                        except:
                            self.state = IMUSeatAngleState.AA_ERROR
                    else:
                        pass

                elif (self.aa.getStateName() == 'CALIBRATION_RUNNING_ZERO'
                    or self.aa.getStateName() == 'CALIBRATION_RUNNING_INCLINED'):
                    self.state = IMUSeatAngleState.WAITING_FOR_CALIBRATION
                    pass

                elif (self.aa.getStateName() == 'CALIBRATION_DONE'):
                    self.state = IMUSeatAngleState.CALIBRATION_DONE

                else:
                    self.state = IMUSeatAngleState.AA_ERROR
                pass


        print('State: ', self.state.name)
        return self.seat_angle


def createClient(clientName,config):
    #create client connected to mqtt broker
    
    #create new instance
    print("MQTT creating new instance named : "+str(clientName))
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
    try:
        ############
        # connect to mqtt broker
        client = createClient(None, config)

        # Will start background thread for async messages read/write
        client.loop_start()

        imu = IMUSeatAngle()

        # This is forcing calibration.
        # imu.initialize_angle_analysis(config)

        def on_message(client, userdata: IMUSeatAngle, message):
            print(client, userdata, message)
            if message.topic == 'config/calib_imu':
                response = (json.loads(message.payload.decode())['calibrationState'])
                if response != True and response != False:
                    response = True
                imu.calib_trigger(response)

        # Set userdata
        client.user_data_set(imu)

        # Set callback
        client.on_message = on_message

        # Subscribe to calibration topics
        client.subscribe('config/calib_imu')
    
        # Main loop
        last_publish = datetime.now()
        while True:
            # Update angle (depending on state...)
            imu.seat_angle = imu.update(config)

            if int(float(datetime.now().timestamp()-last_publish.timestamp())) >= config.getfloat('IMUSeatAngle','publishPeriod'):
                # Publish real angle value
                client.publish('sensors/angle', imu.to_json())
                last_publish = datetime.now()

            if not imu.aa.getStateName() == 'CALIBRATION_DONE':
                time.sleep(config.getfloat('IMUSeatAngle','samplingPeriod'))
            else:
                time.sleep(config.getfloat('IMUSeatAngle','publishPeriod'))
    except:
        pass
    finally:
        client.disconnect()
