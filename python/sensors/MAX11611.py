from smbus2 import SMBus, i2c_msg
import time
import numpy as np


class MAX11611:
    def __init__(self, address=0x35, channel=1):
        self.address = address
        self.channel = channel
        # Open i2c bus
        # I2C channel 1 is connected to the GPIO pins
        self.bus = SMBus(self.channel)
        self.config()
        self.adc_count = 9
        self.values = np.zeros(self.adc_count)

    def config(self):
        """
        bit7 = 1; //Setup
        bit6 = 0; //SEL2 (Alim options)
        bit5 = 0; //SEL1
        bit4 = 0; //SEL0
        bit3 = 1; //0=Internal 1=External clock
        bit2 = 0; //Unipolar
        bit1 = 1; //No action
        bit0 = 0; //Don't-care bit	
        = 0x8A


        bit7 = 0; //Configuration
        bit6 = 0; //SCAN1
        bit5 = 0; //SCAN0
        bit4 = 1; //CS3 //0011 = AIN3 (Va donc scanner de AIN0 AIN8)
        bit3 = 0; //CS2
        bit2 = 0; //CS1
        bit1 = 0; //CS0
        bit0 = 1; //Single-ended
        = 0x11
        """
        if self.connected():
            try:
                msg = i2c_msg.write(self.address, [0x8A, 0x11])
                self.bus.i2c_rdwr(msg)
            except Exception as e:
                print(e)

    def connected(self):
        # Verify if device is present
        try:
            # Will throw an exception if not found
            self.bus.read_byte(self.address)
            return True
        except Exception as e:
            print(e)
            pass  # discard errors that we get when trying to read from empty address

        # Default = not found
        return False

    def read_adc(self):

        if self.connected():
            # Read all 16 channels (2 bytes each)
            msg = i2c_msg.read(self.address, self.adc_count * 2)
            self.bus.i2c_rdwr(msg)

            temp = [x for x in msg]
        
            for i in range(self.adc_count):
                an = int(temp[2*i] & 0x03) << 8 | int(temp[2*i + 1])
                self.values[i] = an
        else:
            self.values = np.zeros(self.adc_count)
        
        return self.values


    def get_values(self):
        return self.values

if __name__ == "__main__":

    adc = MAX11611()
    while True:
        if adc.connected():
            values = adc.read_adc()
            print(values)
        else:
            print('Not connected')
            adc.config()

        # Sleep 1 sec
        time.sleep(1.0)