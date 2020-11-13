import json
import datetime

'''
Raw Data: (sensors/rawData)
{
  "time": 1604434062,
  "ToFSensor": {
    "connected": 0,
    "range": 260
  },
  "flowSensor": {
    "connected": 0,
    "travelX": 0,
    "travelY": 0
  },
  "alarmSensor": {
    "connected": 0,
    "redLedOn": 0,
    "redLedBlink": 0,
    "greenLedOn": 0,
    "greenLedBlink": 0,
    "alternatingLedBlink": 0,
    "motorOn": 0,
    "buttonPressed": 0
  },
  "pressureMat": {
    "threshold": 0,
    "connected": 0,
    "calibrated": 0,
    "matData": [
      0,
      0,
      0,
      0,
      0,
      0,
      0,
      0,
      0
    ]
  },
  "mIMU": {
    "connected": 0,
    "calibrated": 0,
    "accX": 0,
    "accY": 0,
    "accZ": 0,
    "gyroX": 0,
    "gyroY": 0,
    "gyroZ": 0
  },
  "fIMU": {
    "connected": 0,
    "calibrated": 0,
    "accX": 0,
    "accY": 0,
    "accZ": 0,
    "gyroX": 0,
    "gyroY": 0,
    "gyroZ": 0
  }
}
ChairState: (sensors/chairState)
{
  "time": 1604434107,
  "snoozeButton": 0,
  "Travel": {
    "isMoving": 0,
    "lastDistance": 0
  },
  "Pressure": {
    "isSeated": 0,
    "centerOfGravity": {
      "x": 0,
      "y": 0
    },
    "centerOfGravityPerQuadrant": [
      {
        "x": 0,
        "y": 0
      },
      {
        "x": 0,
        "y": 0
      },
      {
        "x": 0,
        "y": 0
      },
      {
        "x": 0,
        "y": 0
      }
    ]
  },
  "Angle": {
    "mIMUAngle": 0,
    "fIMUAngle": 0,
    "seatAngle": 0
  }
}







topic:  config/angle_new_offset
Offset angle to be stored in the database.

'''


class TravelInformation:
    def __init__(self):
        self.isMoving = False
        self.lastDistance = float(0.0)

    def to_dict(self):
        return {
            'isMoving': self.isMoving,
            'lastDistance': self.lastDistance
        }

    def from_dict(self, values: dict):
        if 'isMoving' in values:
            self.isMoving = values['isMoving']
        if 'lastDistance' in values:
            self.lastDistance = values['lastDistance']


class PressureInformation:
    def __init__(self):
        self.isSeated = False
        self.centerOfGravity = {'x': float(0.0), 'y': float(0.0)}
        self.centerOfGravityQ1 = {'x': float(0.0), 'y': float(0.0)}
        self.centerOfGravityQ2 = {'x': float(0.0), 'y': float(0.0)}
        self.centerOfGravityQ3 = {'x': float(0.0), 'y': float(0.0)}
        self.centerOfGravityQ4 = {'x': float(0.0), 'y': float(0.0)}

    def to_dict(self):
        return {
            'isSeated': self.isSeated,
            'centerOfGravity': self.centerOfGravity,
            'centerOfGravityPerQuadrant': [self.centerOfGravityQ1, self.centerOfGravityQ2,
                                           self.centerOfGravityQ3, self.centerOfGravityQ4]
        }

    def from_dict(self, values: dict):
        if 'isSeated' in values:
            self.isSeated = values['isSeated']
        if 'centerOfGravity' in values:
            self.centerOfGravity = values['centerOfGravity']
        if 'centerOfGravityPerQuadrant' in values:
            if len(values['centerOfGravityPerQuadrant']) == 4:
                self.centerOfGravityQ1['x'] = values['centerOfGravityPerQuadrant'][0]['x']
                self.centerOfGravityQ1['y'] = values['centerOfGravityPerQuadrant'][0]['y']
                self.centerOfGravityQ2['x'] = values['centerOfGravityPerQuadrant'][1]['x']
                self.centerOfGravityQ2['y'] = values['centerOfGravityPerQuadrant'][1]['y']
                self.centerOfGravityQ3['x'] = values['centerOfGravityPerQuadrant'][2]['x']
                self.centerOfGravityQ3['y'] = values['centerOfGravityPerQuadrant'][2]['y']
                self.centerOfGravityQ4['x'] = values['centerOfGravityPerQuadrant'][3]['x']
                self.centerOfGravityQ4['y'] = values['centerOfGravityPerQuadrant'][3]['y']


class AngleInformation:
    def __init__(self):
        self.mIMUAngle = 0.0
        self.fIMUAngle = 0.0
        self.seatAngle = 0.0

    def to_dict(self):
        return {
            'mIMUAngle': float(self.mIMUAngle),
            'fIMUAngle': float(self.fIMUAngle),
            'seatAngle': float(self.seatAngle)
        }

    def from_dict(self, values: dict):
        if 'mIMUAngle' in values:
            self.mIMUAngle = values['mIMUAngle']
        if 'fIMUAngle' in values:
            self.fIMUAngle = values['fIMUAngle']
        if 'seatAngle' in values:
            self.seatAngle = values['seatAngle']


class ChairState:
    def __init__(self):
        self.timestamp = int(datetime.datetime.now().timestamp())
        self.snoozeButton = False
        self.Travel = TravelInformation()
        self.Pressure = PressureInformation()
        self.Angle = AngleInformation()

    def to_dict(self):
        return {
            'time': self.timestamp,
            'snoozeButton': self.snoozeButton,
            'Travel': self.Travel.to_dict(),
            'Pressure': self.Pressure.to_dict(),
            'Angle': self.Angle.to_dict()
        }

    def from_dict(self, values: dict):
        if 'time' in values:
            self.timestamp = values['time']
        if 'snoozeButton' in values:
            self.snoozeButton = values['snoozeButton']
        if 'Travel' in values:
            self.Travel.from_dict(values['Travel'])
        if 'Pressure' in values:
            self.Pressure.from_dict(values['Pressure'])
        if 'Angle' in values:
            self.Angle.from_dict(values['Angle'])

    def to_json(self):
        return json.dumps(self.to_dict())

    def from_json(self, data: str):
        values = json.loads(data)
        self.from_dict(values)

    def updateRawData(self, data):
        if 'time' in data:
            self.timestamp = int(datetime.datetime.now().timestamp())
        # print(data)


