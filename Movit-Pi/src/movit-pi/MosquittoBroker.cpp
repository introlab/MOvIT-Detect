#include <iostream>
#include <cstdio>
#include <string>
#include "rapidjson/document.h"
#include "rapidjson/writer.h"
#include "rapidjson/stringbuffer.h"
#include "MosquittoBroker.h"


using rapidjson::Document;
using rapidjson::StringBuffer;
using rapidjson::Value;
using rapidjson::Writer;


/* Subscibed Topics */
const char *ALARM_TOPIC = "data/set_alarm";
//const char *REQUIRED_ANGLE_TOPIC = "data/required_back_rest_angle";
//const char *REQUIRED_PERIOD_TOPIC = "data/required_period";
const char *REQUIRED_DURATION_TOPIC = "data/required_duration";
const char *GOAL_CHANGED_TOPIC = "goal/update_data";
const char *CALIB_PRESSURE_MAT_TOPIC = "config/calib_pressure_mat";
const char *CALIB_IMU_TOPIC = "config/calib_imu";
const char *CALIB_IMU_WITH_OFFSET_TOPIC = "config/calib_imu_offset";
const char *NOTIFICATIONS_SETTINGS_TOPIC = "config/notifications_settings";
const char *SELECT_WIFI_TOPIC = "config/wifi";


/* Published Topics */
const char *CURRENT_IS_WIFI_CONNECTED_TOPIC = "data/current_is_wifi_connected";
const char *HEARTBEAT_TOPIC = "heartbeat/embedded";
const char *SENSOR_RAW_DATA_TOPIC = "sensors/rawData";
const char *SENSOR_CHAIR_STATE_TOPIC = "sensors/chairState";
const char *FSM_ANGLE_TOPIC = "fsm/angle";
const char *FSM_SEATING_TOPIC = "fsm/seating";
const char *FSM_TRAVEL_TOPIC = "fsm/travel";
const char *FSM_NOTIFICATION_TOPIC = "fsm/notification";
const char *ANGLE_NEW_OFFSET_TOPIC = "config/angle_new_offset";

const char *EXCEPTION_MESSAGE = "Exception thrown by %s()\n";

MosquittoBroker::MosquittoBroker(const char *id) : mosquittopp(id)
{
    mosqpp::lib_init();

    const char *username = "admin";
    const char *password = "movitplus";

    const char *host = "localhost";
    const int keepAlive = 60;
    const int port = 1883;

    if (username_pw_set(username, password) != MOSQ_ERR_SUCCESS)
    {
        printf("Failed to configure username and password for a mosquitto instance.\n");
        throw;
    }

    connect_async(host, port, keepAlive);

    if (loop_start() != MOSQ_ERR_SUCCESS)
    {
        printf("Failed to to start a new thread to process network traffic.\n");
        throw;
    }
}

MosquittoBroker::~MosquittoBroker()
{
    disconnect();

    if (loop_stop() != MOSQ_ERR_SUCCESS)
    {
        printf("Failed to stop the network thread previously created.\n");
    }

    mosqpp::lib_cleanup();
}

void MosquittoBroker::on_connect(int rc)
{
    printf("Connected with code %d.\n", rc);
    if (rc == 0)
    {
        subscribe(NULL, ALARM_TOPIC);
        subscribe(NULL, CALIB_IMU_WITH_OFFSET_TOPIC);
        subscribe(NULL, GOAL_CHANGED_TOPIC);
        subscribe(NULL, CALIB_PRESSURE_MAT_TOPIC);
        subscribe(NULL, CALIB_IMU_TOPIC);
        subscribe(NULL, NOTIFICATIONS_SETTINGS_TOPIC);
        subscribe(NULL, SELECT_WIFI_TOPIC);
        PublishMessage("status", "1", 1, true);
    }
}

void MosquittoBroker::on_publish(int mid)
{
    // printf("Message published with id %d.\n", mid);
    // uncomment for debug
}

void MosquittoBroker::on_subcribe(int mid, int qos_count, const int *granted_qos)
{
    printf("Subscription succeeded with id %d.\n", mid);
}

void MosquittoBroker::on_message(const mosquitto_message *msg)
{
    std::string message;
    std::string topic;

    if (msg->payload != NULL)
    {
        message = reinterpret_cast<char *>(msg->payload);
    }
    else
    {
        message = "";
    }
    topic = msg->topic;

    if (topic == SELECT_WIFI_TOPIC)
    {
        _wifiChanged = true;
        _wifiInformation = message;
    }
    else if(topic == GOAL_CHANGED_TOPIC) {

        try
        {
            int num = 0;
            std::string arr[5] = {"","","","",""};
            splitStringWithDelemiter(message,":",arr,&num);
            if(num == 3) {
                _frequency = std::stoi(arr[0]);
                _duration = std::stoi(arr[1]);
                _angle = std::stoi(arr[2]);
                _GoalChanged = true;
            }
        }
        catch (const std::exception &e)
        {
            printf(EXCEPTION_MESSAGE, e.what());
        }
    }
    
    else if (topic == CALIB_IMU_WITH_OFFSET_TOPIC)
    {
        try
        {
            int num = 0;
            std::string arr[5] = {"","","","",""};
            splitStringWithDelemiter(message,":",arr,&num);
            if(num == 2) {
                _mIMUOffset = std::stoi(arr[0]);
                _fIMUOffset = std::stoi(arr[1]);
                _OffsetChanged = true;
            }
        }
        catch (const std::exception &e)
        {
            printf(EXCEPTION_MESSAGE, e.what());
        }
    }
    else if (topic == CALIB_PRESSURE_MAT_TOPIC)
    {
        try
        {
            _calibPressureMatRequired = std::stoi(message);
        }
        catch (const std::exception &e)
        {
            printf(EXCEPTION_MESSAGE, e.what());
            printf("Setting _calibPressureMatRequired to false\n");
            _calibPressureMatRequired = false;
        }
    }
    else if (topic == CALIB_IMU_TOPIC)
    {
        try
        {
            _calibIMURequired = std::stoi(message);
        }
        catch (const std::exception &e)
        {
            printf(EXCEPTION_MESSAGE, e.what());
            printf("Setting _requiredDuration to 0\n");
            _calibIMURequired = false;
        }
    }
    else if (topic == NOTIFICATIONS_SETTINGS_TOPIC)
    {
        try
        {
            int num = 0;
            std::string arr[5] = {"","","","",""};
            splitStringWithDelemiter(message,":",arr,&num);
            if(num == 4) {
                _shouldBlink = std::stoi(arr[0]);
                _shouldVibrate = std::stoi(arr[1]);
                _snoozeTime = std::stoi(arr[2]);
                _isEnabled = std::stoi(arr[3]);
                _isNotificationsSettingsChanged = true;
            }
        }
        catch (const std::exception &e)
        {
            printf(EXCEPTION_MESSAGE, e.what());
        }
    }
}

void MosquittoBroker::SendHeartbeat(const std::string datetime)
{
    std::string strMsg = "{\"datetime\":" + datetime + "}";

    PublishMessage(HEARTBEAT_TOPIC, strMsg);
}

void MosquittoBroker::SendSensorsData(SensorData sd)
{
    std::string strMsg = "";
    strMsg ="{\
                \"time\" : "+std::to_string(sd.time)+",\
                \"ToFSensor\" : {\
                    \"connected\" : "+std::to_string(sd.tofConnected)+",\
                    \"range\" : "+std::to_string(sd.tofRange)+"\
                },\
                \"flowSensor\" : {\
                    \"connected\" : "+std::to_string(sd.flowConnected)+",\
                    \"travelX\" : "+std::to_string(sd.flowTravelX)+",\
                    \"travelY\" : "+std::to_string(sd.flowTravelY)+"\
                },\
                \"alarmSensor\" : {\
                    \"connected\" : "+std::to_string(sd.alarmConnected)+",\
                    \"redLedOn\" : "+std::to_string(sd.alarmRedLedOn)+",\
                    \"greenLedOn\" : "+std::to_string(sd.alarmGreenLedOn)+",\
                    \"motorOn\" : "+std::to_string(sd.alarmDCMotorOn)+",\
                    \"buttonPressed\" : "+std::to_string(sd.alarmButtonPressed)+"\
                },\
                \"pressureMat\" : {\
                    \"connected\" : "+std::to_string(sd.matConnected)+",\
                    \"matData\" : [ "+std::to_string(sd.matData[0])+", "+std::to_string(sd.matData[1])+", "+std::to_string(sd.matData[2])+", "+std::to_string(sd.matData[3])+", "+std::to_string(sd.matData[0])+", "+std::to_string(sd.matData[5])+", "+std::to_string(sd.matData[6])+", "+std::to_string(sd.matData[7])+", "+std::to_string(sd.matData[8])+" ]\
                },\
                \"mIMU\" : {\
                    \"connected\" : "+std::to_string(sd.mIMUConnected)+",\
                    \"calibrated\" : "+std::to_string(sd.mIMUCalibrated)+",\
                    \"accX\" : "+std::to_string(sd.mIMUAccX)+",\
                    \"accY\" : "+std::to_string(sd.mIMUAccY)+",\
                    \"accZ\" : "+std::to_string(sd.mIMUAccZ)+",\
                    \"gyroX\" : "+std::to_string(sd.mIMUGyroX)+",\
                    \"gyroY\" : "+std::to_string(sd.mIMUGyroY)+",\
                    \"gyroZ\" : "+std::to_string(sd.mIMUGyroZ)+"\
                },\
                \"fIMU\" : {\
                    \"connected\" : "+std::to_string(sd.fIMUConnected)+",\
                    \"calibrated\" : "+std::to_string(sd.fIMUCalibrated)+",\
                    \"accX\" : "+std::to_string(sd.fIMUAccX)+",\
                    \"accY\" : "+std::to_string(sd.fIMUAccY)+",\
                    \"accZ\" : "+std::to_string(sd.fIMUAccZ)+",\
                    \"gyroX\" : "+std::to_string(sd.fIMUGyroX)+",\
                    \"gyroY\" : "+std::to_string(sd.fIMUGyroY)+",\
                    \"gyroZ\" : "+std::to_string(sd.fIMUGyroZ)+"\
                }\
            }";
    PublishMessage(SENSOR_RAW_DATA_TOPIC, strMsg);
}

void MosquittoBroker::SendChairState(ChairState cs)
{
    std::string strMsg = "";
    strMsg ="{\
                \"time\" : "+std::to_string(cs.time)+",\
                \"snoozeButton\" : "+std::to_string(cs.button)+",\
                \"Travel\" : {\
                    \"isMoving\" : "+std::to_string(cs.isMoving)+",\
                    \"lastDistance\" : "+std::to_string(cs.lastDistance)+"\
                },\
                \"Pressure\" : {\
                    \"isSeated\" : "+std::to_string(cs.isSeated)+",\
                    \"centerOfGravity\" : { \"x\":"+std::to_string(cs.centerOfGravity.x)+", \"y\":"+std::to_string(cs.centerOfGravity.y)+" },\
                    \"centerOfGravityPerQuadrant\" : [{\"x\":"+std::to_string(cs.centerOfGravityPerQuadrant[0].x)+", \"y\":"+std::to_string(cs.centerOfGravityPerQuadrant[0].y)+"}, {\"x\":"+std::to_string(cs.centerOfGravityPerQuadrant[1].x)+", \"y\":"+std::to_string(cs.centerOfGravityPerQuadrant[1].y)+"}, {\"x\":"+std::to_string(cs.centerOfGravityPerQuadrant[2].x)+", \"y\":"+std::to_string(cs.centerOfGravityPerQuadrant[2].y)+"}, {\"x\":"+std::to_string(cs.centerOfGravityPerQuadrant[3].x)+", \"y\":"+std::to_string(cs.centerOfGravityPerQuadrant[3].y)+"}]\
                },\
                \"Angle\" : {\
                    \"mIMUAngle\" : "+std::to_string(cs.mIMUAngle)+",\
                    \"fIMUAngle\" : "+std::to_string(cs.fIMUAngle)+",\
                    \"seatAngle\" : "+std::to_string(cs.seatAngle)+"\
                }\
            }";
    PublishMessage(SENSOR_CHAIR_STATE_TOPIC, strMsg);
}

void MosquittoBroker::SendAngleOffset(int mIMUOffset, int fIMUOffset)
{
    std::string strMsg = "";
    strMsg ="{\
                \"mIMUOffset\" : "+std::to_string(mIMUOffset)+",\
                \"fIMUOffset\" : "+std::to_string(fIMUOffset)+"\
            }";
    PublishMessage(ANGLE_NEW_OFFSET_TOPIC, strMsg);
}

void MosquittoBroker::SendNotificationFSM(NotificationFSM notificationFSM)
{
    std::string strMsg = "";
    strMsg ="{\
                \"time\" : "+std::to_string(notificationFSM.getCurrentTime())+",\
                \"elapsed\" : "+std::to_string(notificationFSM.getElapsed())+",\
                \"event\" : \""+notificationFSM.getReason()+"\",\
                \"stateNum\" : "+std::to_string(notificationFSM.getCurrentState())+",\
                \"stateName\" : \""+notificationFSM.getCurrentStateName()+"\"\
            }";
    PublishMessage(FSM_NOTIFICATION_TOPIC, strMsg);
}

void MosquittoBroker::SendAngleFSM(AngleFSM angleFSM) {
    std::string eventState = "";
    std::string strMsg = "";
    if(angleFSM.getCurrentState() == static_cast<int>(AngleState::ANGLE_STARTED)) {
        eventState = "Started";
        strMsg ="{\
                \"time\" : "+std::to_string(angleFSM.getCurrentTime())+",\
                \"elapsed\" : "+std::to_string(angleFSM.getElapsedTime())+",\
                \"event\" : \""+eventState+"\",\
                \"stateNum\" : "+std::to_string(angleFSM.getCurrentState())+",\
                \"stateName\" : \""+angleFSM.getCurrentStateName()+"\"\
            }";
    } else if(angleFSM.getCurrentState() == static_cast<int>(AngleState::ANGLE_STOPPED)) {
        eventState = "Stopped";
        int result[5];
        angleFSM.getTimePerAngle(result);
        strMsg ="{\
                \"time\" : "+std::to_string(angleFSM.getCurrentTime())+",\
                \"elapsed\" : "+std::to_string(angleFSM.getElapsedTime())+",\
                \"event\" : \""+eventState+"\",\
                \"stateNum\" : "+std::to_string(angleFSM.getCurrentState())+",\
                \"lessThanZero\" : "+std::to_string(result[0])+",\
                \"zeroToFifteen\" : "+std::to_string(result[1])+",\
                \"fifteenToThirty\" : "+std::to_string(result[2])+",\
                \"thirtyToFourtyfive\" : "+std::to_string(result[3])+",\
                \"fourtyfiveAndMore\" : "+std::to_string(result[4])+",\
                \"average\" : "+std::to_string(angleFSM.getAngleAverage())+",\
                \"requiredDuration\" : "+std::to_string(angleFSM.getTargetDuration())+",\
                \"requiredAngle\" : "+std::to_string(angleFSM.getTargetAngle())+",\
                \"stateName\" : \""+angleFSM.getCurrentStateName()+"\"\
            }";
    } else {
        eventState = "Other";
        strMsg ="{\
                \"time\" : "+std::to_string(angleFSM.getCurrentTime())+",\
                \"elapsed\" : "+std::to_string(angleFSM.getElapsedTime())+",\
                \"event\" : \""+eventState+"\",\
                \"stateNum\" : "+std::to_string(angleFSM.getCurrentState())+",\
                \"stateName\" : \""+angleFSM.getCurrentStateName()+"\"\
            }";
    }

    PublishMessage(FSM_ANGLE_TOPIC, strMsg);
}

void MosquittoBroker::SendTravelFSM(TravelFSM travelFSM) {
    std::string eventState = "";
    if(travelFSM.getCurrentState() == static_cast<int>(TravelState::TRAVEL_STARTED)) {
        eventState = "Started";
    } else if(travelFSM.getCurrentState() == static_cast<int>(TravelState::TRAVEL_STOPPED)) {
        eventState = "Stopped";
    } else {
        eventState = "Other";
    }
    std::string strMsg = "";
    strMsg ="{\
                \"time\" : "+std::to_string(travelFSM.getCurrentTime())+",\
                \"elapsed\" : "+std::to_string(travelFSM.getElapsedTime())+",\
                \"event\" : \""+eventState+"\",\
                \"stateNum\" : "+std::to_string(travelFSM.getCurrentState())+",\
                \"stateName\" : \""+travelFSM.getCurrentStateName()+"\"\
            }";
    PublishMessage(FSM_TRAVEL_TOPIC, strMsg);
}

void MosquittoBroker::SendSeatingFSM(SeatingFSM seatingFSM) {
    std::string eventState = "";
    if(seatingFSM.getCurrentState() == static_cast<int>(SeatingState::SEATING_STARTED)) {
        eventState = "Started";
    } else if(seatingFSM.getCurrentState() == static_cast<int>(SeatingState::SEATING_STOPPED)) {
        eventState = "Stopped";
    } else {
        eventState = "Other";
    }
    std::string strMsg = "";
    strMsg ="{\
                \"time\" : "+std::to_string(seatingFSM.getCurrentTime())+",\
                \"elapsed\" : "+std::to_string(seatingFSM.getElapsedTime())+",\
                \"event\" : \""+eventState+"\",\
                \"stateNum\" : "+std::to_string(seatingFSM.getCurrentState())+",\
                \"stateName\" : \""+seatingFSM.getCurrentStateName()+"\"\
            }";
    PublishMessage(FSM_SEATING_TOPIC, strMsg);
}

void MosquittoBroker::PublishMessage(const char *topic, const std::string message, int qos, bool retain)
{
    if (message.empty())
    {
        return;
    }
    publish(NULL, topic, message.length(), message.c_str(), qos, retain);
}

void MosquittoBroker::splitStringWithDelemiter(std::string toSplit, std::string delimiter, std::string *elements, int *numElem) {
        size_t pos = 0;
        int i = 0;
        std::string token;
        while ((pos = toSplit.find(delimiter)) != std::string::npos) {
                token = toSplit.substr(0, pos);
                toSplit.erase(0, pos + delimiter.length());
                elements[i] = token;
                i++;
        }
        elements[i] = toSplit;
        *numElem = i+1;
}



    bool MosquittoBroker::CalibIMURequired() {
      return _calibIMURequired;
    }

    void MosquittoBroker::SetCalibIMURequired(bool required) {
      _calibIMURequired = required;
    }

    bool MosquittoBroker::GoalHasChanged() {
      return _GoalChanged;
    }

    void MosquittoBroker::SetGoalHasChanged(bool changed) {
      _GoalChanged = changed;
    }

    void MosquittoBroker::getGoal(int *frequency, int *duration, int *angle) {
      *frequency = _frequency;
      *duration = _duration;
      *angle = _angle;
    }

    bool MosquittoBroker::NotificationHasChanged() {
      return _isNotificationsSettingsChanged;
    }

    void MosquittoBroker::SetNotificationHasChanged(bool changed) {
      _isNotificationsSettingsChanged = changed;
    }

    void MosquittoBroker::getNotificationSettings(bool *ledBlinkingEnable, bool *vibrationEnabled, int *snoozeTime, bool *enabled) {
      *ledBlinkingEnable = _shouldBlink;
      *vibrationEnabled = _shouldVibrate;
      *snoozeTime = _snoozeTime;
      *enabled = _isEnabled;
    }


    bool MosquittoBroker::offsetChanged() {
      return _OffsetChanged;
    }

    void MosquittoBroker::setOffsetChanged(bool changed) {
      _OffsetChanged = changed;
    }

    void MosquittoBroker::getOffsets(int *mIMUOffset, int *fIMUOffset) {
      *mIMUOffset = _mIMUOffset;
      *fIMUOffset = _fIMUOffset;
    }
