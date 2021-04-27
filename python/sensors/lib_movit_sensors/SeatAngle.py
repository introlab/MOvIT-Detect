from datetime import datetime
import json

class SeatAngle:
    def __init__(self):
        self.seat_angle = 0.0
        self.seat_angle_offset = 0.0
        self.last_update = datetime.now()

    def update(self) -> float:
        self.last_update = datetime.now()
        return False

    def connected(self) -> bool:
        return False
    
    def calibrated(self):
        return False

    def to_dict(self) -> dict:
        result = {
            'name': self.__class__.__name__,
            'timestamp': int(self.last_update.timestamp()),
            'connected': self.connected(),
            'calibrated': self.calibrated(),
            'seat_angle': self.seat_angle,
            'seat_angle_offset': self.seat_angle_offset
        }
        return result

    def to_json(self):
        return json.dumps(self.to_dict())


if __name__ == "__main__":
    # Testing Angle
    angle = SeatAngle()
    print(angle.to_json())