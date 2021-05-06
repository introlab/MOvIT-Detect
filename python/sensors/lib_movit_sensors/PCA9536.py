import smbus2 as smbus

class pca9536:

    REG_INPUT = 0x00
    REG_OUTPUT = 0x01
    REG_POLARITY = 0x02
    REG_CONFIG = 0x03

    IO_DC_MOTOR = 0
    IO_PUSH_BUTTON = 1
    IO_RED_LED = 2
    IO_GREEN_LED = 3

    IO_NON_INVERTED = 0
    IO_INVERTED = 1

    def __init__(self, address=0x41, bus=1):
        self.address = address
        self.bus = smbus.SMBus(bus)
        self.reset()
    
    def __del__(self):
        self.reset()

    def connected(self):
        # Verify if device is present
        try:
            # Will throw an exception if not found
            self.bus.read_byte(self.address)
            return True
        except Exception as e:
            print(e)
            self.reset()
            pass  # discard errors that we get when trying to read from empty address

        # Default = not found
        return False


    def reset(self):
        # initialize registers
        # if self.connected():
        # Polarity not inverted
        try:
            self.bus.write_byte_data(self.address, pca9536.REG_POLARITY, 0x00)

            # Set pins input/output 
            self.bus.write_byte_data(self.address, pca9536.REG_CONFIG, 0xF2)

            # Set outputs
            self.write_outputs(0x00)

        except Exception as e:
            print(e)  

    def inputs_state(self):
        if self.connected():
            return self.bus.read_byte_data(self.address, pca9536.REG_INPUT)
        else:
            # default value
            return 0xFF

    def outputs_state(self):
        if self.connected():
            return self.bus.read_byte_data(self.address, pca9536.REG_OUTPUT)
        else:
            # default value
            return 0x00

    def write_outputs(self, value):
        if self.connected():
            self.bus.write_byte_data(self.address, pca9536.REG_OUTPUT, value)

    def digitalWrite(self, pin, value):
        if pin > 3:
            return
        
        state = self.outputs_state()
        if value:
            state = state | (0x01 << pin)
        else:
            state = state & ~(0x01 << pin)
        self.write_outputs(state)
        
        return state
    
    def digitalRead(self, pin):
        if pin > 3:
            return  
        state = self.inputs_state()
        return int(state >> pin & 0x01)

    def set_motor(self, enabled):
        self.digitalWrite(pca9536.IO_DC_MOTOR, int(enabled))

    def get_motor(self):
        return (self.outputs_state() >> pca9536.IO_DC_MOTOR) & 0x01

    def set_green_led(self, enabled):
        self.digitalWrite(pca9536.IO_GREEN_LED, int(enabled))

    def get_green_led(self):
        return (self.outputs_state() >> pca9536.IO_GREEN_LED) & 0x01

    def toggle_green_led(self):
        self.set_green_led(not self.get_green_led())

    def set_red_led(self, enabled):
        self.digitalWrite(pca9536.IO_RED_LED, int(enabled))

    def get_red_led(self):
        return (self.outputs_state() >> pca9536.IO_RED_LED) & 0x01

    def toggle_red_led(self):
        self.set_red_led(not self.get_red_led())

    def get_button(self):
        return self.digitalRead(pca9536.IO_PUSH_BUTTON)

    


if __name__ == "__main__":
    import time
    pca = pca9536()
    while True:
        print('button:',  pca.get_button())
        time.sleep(1)
        pca.toggle_green_led()
        pca.toggle_red_led()
        pca.set_motor(1)
        print('motor:',  pca.get_motor())
        print('green_led:',  pca.get_green_led())
        print('red_led:',  pca.get_red_led())
        time.sleep(1)
        pca.set_motor(0)
