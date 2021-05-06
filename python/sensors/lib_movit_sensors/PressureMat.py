import json
import numpy as np
from datetime import datetime
'''
Generic pressure mat implementation.
Derive from this class as needed.
'''


class PressureMat:
    def __init__(self, nb_sensors: int):
        self.nb_sensors = nb_sensors
        self.values = np.zeros(nb_sensors)
        self.offsets = np.zeros(nb_sensors)
        self.coordinates = np.array([np.zeros(2) for i in range(nb_sensors)])
        self.last_update = datetime.now()

        # Call init functions
        self.init_coordinates()
        self.init_values()
        self.init_offsets()

    def init_coordinates(self):
        pass

    def init_offsets(self):
        pass

    def init_values(self):
        pass

    def update(self):
        self.last_update = datetime.now()

    def connected(self):
        return False

    def calculate_center_of_pressure(self):
        value_sum = np.sum(self.values)
        if value_sum != 0:
            cop = (self.values - self.offsets).dot(self.coordinates) / value_sum
        else:
            cop = np.zeros(2)
        return cop, value_sum

    def to_json(self):
        cop, value_sum = self.calculate_center_of_pressure()
        result = {
            'name': self.__class__.__name__,
            'timestamp': int(self.last_update.timestamp()),
            'connected': self.connected(),
            'values': self.values.tolist(),
            'offsets': self.offsets.tolist(),
            'coordinates': self.coordinates.tolist(),
            'cop': {'x': float(cop[0]), 'y': float(cop[1]), 'sum': float(value_sum)}
        }
        return json.dumps(result)


class TestPressurePlate(PressureMat):
    def __init__(self):
        PressureMat.__init__(self, 9)

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

    def init_values(self):
        self.values = np.zeros(self.nb_sensors)

    def update(self):
        # Read sensor
        pass


if __name__ == "__main__":
    # Testing Mat
    mat = TestPressurePlate()
    print(mat.calculate_center_of_pressure())
    print(mat.to_json())


