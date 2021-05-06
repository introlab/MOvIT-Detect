# Copyright (c) 2021 Adrien Pajon (adrien.pajon@gmail.com)
# 
# This software is released under the MIT License.
# https://opensource.org/licenses/MIT


from .MPU6050 import mpu6050

class mpu6050_safe(mpu6050):
    def wake_up(self):
        if self.connected():
            if self.sleeping():
                # Wake up the MPU-6050 since it starts in sleep mode
                self.bus.write_byte_data(self.address, self.PWR_MGMT_1, self.bus.read_byte_data(self.address, self.PWR_MGMT_1) - 0x40)

                # Setup RANGE
                self.set_accel_range(mpu6050.ACCEL_RANGE_2G)
                self.set_gyro_range(mpu6050.GYRO_RANGE_250DEG)
        else:
            print('mpu6050_safe.wake_up not connected addr:', self.address)

    def sleeping(self):
        if self.connected():
            return (self.bus.read_byte_data(self.address, self.PWR_MGMT_1)>>6)==1
        else:
            print('mpu6050_safe.sleeping not connected addr:', self.address)
            return False