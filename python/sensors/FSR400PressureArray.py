from PressureMat import PressureMat
from MAX11611 import MAX11611
import numpy as np
import time


class FSR400PressureArray(PressureMat):
    def __init__(self):
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
            self.values = self.adc.read_adc()


if __name__ == "__main__":
    mat = FSR400PressureArray()

    while True:    
        
        if mat.connected():
            mat.update()

        print(mat.calculate_center_of_pressure())
        print(mat.to_json())
        time.sleep(1)
