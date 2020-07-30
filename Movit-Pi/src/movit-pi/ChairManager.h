#ifndef CHAIR_MANAGER_H
#define CHAIR_MANAGER_H

#include "MosquittoBroker.h"
#include "Utils.h"
#include "Timer.h"
#include "DeviceManager.h"
#include "NotificationFSM.h"
#include "SeatingFSM.h"
#include "TravelFSM.h"
#include "AngleFSM.h"
#include "SecondsCounter.h"
#include <cmath>

#include <string>
#include <unistd.h>
#include <chrono>

const Coord_t POSITION_LOOKUP[9] = {{4.0f,4.0f}, {4.0f,0.0f}, {4.0f,-4.0f}, {0.0f,4.0f}, {0.0f,0.0f}, {0.0f,-4.0f}, {-4.0f,4.0f}, {-4.0f,0.0f}, {-4.0f,-4.0f}};

class ChairManager
{
  public:
    ChairManager(MosquittoBroker *mosquittoBroker, DeviceManager *deviceManager);
    ~ChairManager();

    void UpdateDevices();
    void ReadFromServer();
    void CheckNotification();
    void displaySensorData(SensorData sensorData);
    void displayChairState(ChairState chairState);
    void setAngleOffset(int fixedOffset, int mobileOffset);
    int getQuadrant(float y, float x);

  private:
    float calculatemIMUAngle(SensorData sd);
    float calculatefIMUAngle(SensorData sd);
    float calculateSeatAngle(ChairState cs);

    int fIMUOffset = 0;
    int mIMUOffset = 0;

    Coord_t calculateCenterOfGravity(SensorData sd); 
    void calculateCenterOfGravityPerQuadrant(SensorData sd, Coord_t *quad);
    bool verifyIfUserIsSeated() { return _deviceManager->IsUserSeated(); }

    uint32_t calculateDistance(SensorData sd);

    //Alarm *_alarm;
    MosquittoBroker *_mosquittoBroker;
    DeviceManager *_deviceManager;

    pressure_mat_data_t _pressureMatData;
    tilt_settings_t _tiltSettings;
    SensorData sensorData;
    ChairState chairState;
    SeatingFSM seatingFSM = SeatingFSM();
    TravelFSM travelFSM = TravelFSM();
    AngleFSM angleFSM = AngleFSM();
    NotificationFSM notificationFSM = NotificationFSM();
    int lastNotificationState = -1;
    std::chrono::high_resolution_clock::time_point mIMUStart = std::chrono::high_resolution_clock::now();
    std::chrono::high_resolution_clock::time_point mIMUFinish = std::chrono::high_resolution_clock::now();
    std::chrono::high_resolution_clock::time_point fIMUStart = std::chrono::high_resolution_clock::now();
    std::chrono::high_resolution_clock::time_point fIMUFinish = std::chrono::high_resolution_clock::now();
    float mobileAngleUncorrected = 0;
    float fixedAngleUncorrected = 0;
    int seatAngleUncorrected = 0;

};

#endif // CHAIR_MANAGER_H
