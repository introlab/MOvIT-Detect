from SeatAngle import SeatAngle
from MPU6050 import mpu6050
import numpy as np
import time
import paho.mqtt.client as mqtt
import json
import configparser
import os
import math
from lib_movit_sensors.AngleAnalysis import AngleAnalysis 

class IMUSeatAngle(SeatAngle):
    def __init__(self):
        SeatAngle.__init__(self)
        self.fixed_imu = mpu6050(address=0x68)
        self.mobile_imu = mpu6050(address=0x69)
        self.fixed_imu_data = self.fixed_imu.get_all_data()
        self.mobile_imu_data = self.mobile_imu.get_all_data()


    def connected(self) -> bool:
        return self.fixed_imu.connected() and self.mobile_imu.connected()
    
    def calibrated(self):
        return False

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

    def update(self) -> float:
        # will update timestamp
        super().update()

        # Reset IMU every time ?
        # This way we are sure the device is reinitialized if unplugged/plugged
        self.fixed_imu.reset()
        self.mobile_imu.reset()

        self.fixed_imu_data = self.fixed_imu.get_all_data(raw=True)
        self.mobile_imu_data = self.mobile_imu.get_all_data(raw=True)

        # if self.connected():
        #     self.seat_angle = self.calculate_angle()
        # else:
        #     self.seat_angle = 0

        return True


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
    
    # Make sure working directory is the directory with the current file
    abspath = os.path.abspath(__file__)
    dname = os.path.dirname(abspath)
    os.chdir(dname)

    ############
    # import config file
    config = configparser.ConfigParser()

    print("opening configuration file : config.cfg")
    ret = config.read('config.cfg')

    ############
    # connect to mqtt broker
    client = createClient("LoggerAngle", config)


    aa = AngleAnalysis()

    # Start calibration
    aa.intializeIMU(client, config)

    imu = IMUSeatAngle()




    while not aa.askAtZero:
        pass
        time.sleep(0.1)
    else:
        try:
            input("Please, set the Seat at Zero\n")
        except (Exception, KeyboardInterrupt):
            pass
        finally:
            print("Seat at Zero !")
            aa.isAtZero = True

    while aa.askAtZero:
        imu.update()
        client.publish(config.get('MQTT', 'topic_publish'), imu.to_json())
        time.sleep(1)

    while not aa.askInclined:
        pass
        time.sleep(0.1)
    else:
        try:
            input("Please, Tilt the Seat\n")
        except (Exception, KeyboardInterrupt):
            pass
        finally:
            aa.isInclined = True
            print("Seat is Tilted !")
    
    while aa.askInclined:
        imu.update()
        client.publish(config.get('MQTT', 'topic_publish'), imu.to_json())
        time.sleep(1)


    # Wait for calibration calculation
    while not aa.isRotWorld:
        time.sleep(1)
    else:
        aa.startGetAngle(client, config)

    # Main loop
    while True:
        imu.update()
        client.publish(config.get('MQTT', 'topic_publish'), imu.to_json())
        # Publish real angle value
        # client.publish('sensors/angle', angle.to_json())
        result = aa.getAngleAnalysis()

        imu.seat_angle = result['angleSiege']
        client.publish('sensors/angle', imu.to_json())

        time.sleep(1)
