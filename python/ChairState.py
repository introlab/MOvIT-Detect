import paho.mqtt.client as mqtt
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

AngleFSM: (fsm/angle)
{
  "time": 1604434147,
  "elapsed": 0,
  "event": "Other",
  "stateNum": 0,
  "stateName": "INIT"
}
TravelFSM: (fsm/travel)
{
  "time": 1604434147,
  "elapsed": 0,
  "event": "Other",
  "stateNum": 0,
  "stateName": "INIT"
}
SeatingFSM: (fsm/seating)
{
  "time": 1604434147,
  "elapsed": 0,
  "event": "Other",
  "stateNum": 0,
  "stateName": "INIT"
}
NotificationFSM: (fsm/notification)
{
  "time": 1604434147,
  "elapsed": 0,
  "event": "Other",
  "stateNum": 0,
  "stateName": "INIT"
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

    def updateRawData(self, data):
        if 'time' in data:
            self.timestamp = int(datetime.datetime.now().timestamp())
        # print(data)


# Global chair state
chair = ChairState()


# The callback for when the client receives a CONNACK response from the server.
def on_connect(client, userdata, flags, rc):
    print("Connected with result code " + str(rc))

    # Subscribing in on_connect() means that if we lose the connection and
    # reconnect then subscriptions will be renewed.
    client.subscribe("sensors/rawData")


# The callback for when a PUBLISH message is received from the server.
def on_message(client, userdata, msg):
    # print(msg.topic+" "+str(msg.payload))

    if 'sensors/rawData' in msg.topic:
        rawData = json.loads(msg.payload)
        # print(state)
        chair.updateRawData(rawData)

        # publish chair state
        state = chair.to_dict()
        print(state)
        client.publish('sensors/chairState', json.dumps(chair.to_dict()))


if __name__ == "__main__":
    print('Starting ChairState')

    client = mqtt.Client()
    client.on_connect = on_connect
    client.on_message = on_message

    client.username_pw_set('admin', 'movitplus')
    client.connect("192.168.3.214", 1883, 60)

    # Blocking call that processes network traffic, dispatches callbacks and
    # handles reconnecting.
    # Other loop*() functions are available that give a threaded interface and a
    # manual interface.

    client.loop_forever()
