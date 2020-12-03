from SeatAngle import SeatAngle
from MPU6050 import mpu6050
import numpy as np
import time
import paho.mqtt.client as mqtt
import json
import math

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
        # will update timestampe
        super().update()

        # Reset IMU every time ?
        # This way we are sure the device is reinitialized if unplugged/plugged
        self.fixed_imu.reset()
        self.mobile_imu.reset()

        self.fixed_imu_data = self.fixed_imu.get_all_data(raw=True)
        self.mobile_imu_data = self.mobile_imu.get_all_data(raw=True)

        if self.connected():
            self.seat_angle = self.calculate_angle()
        else:
            self.seat_angle = 0

        return True

if __name__ == "__main__":
    
    with open('sensors/config.json', mode='r') as f:
        data = f.read()
        config = json.loads(data)
        server_config = config['server']
        print(config)

    angle = IMUSeatAngle()

    # Create MQTT client
    client = mqtt.Client('IMUSeatAngle MQTT Client')
    client.username_pw_set(server_config['username'], server_config['password'])
    client.connect(host=server_config['hostname'], port=server_config['port'])
    
    while True:    
        angle.update()     
        client.publish('sensors/angle', angle.to_json())
        time.sleep(1)