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

// Embarqué à back-end
// data/current_back_rest_angle: entier (angle en degrés) ce qui est envoyé est l'angle de début et l'angle de fin de la bascule
// data/current_center_of_pressure: Coordonées en entier. Format: X:posX,Y:posy
// data/current_is_someone_there: bool (1 = on, 0 = off)
// data/current_chair_speed: float (m/s)

// Back-end à embarqué
// data/required_back_rest_angle: entier (angle en degrés)
// data/required_period: entier (minutes)
// data/required_duration: entier (seconde)
// data/set_alarm: entier (1 = on, 0 = off)

const char *ALARM_TOPIC = "data/set_alarm";
const char *REQUIRED_ANGLE_TOPIC = "data/required_back_rest_angle";
const char *REQUIRED_PERIOD_TOPIC = "data/required_period";
const char *REQUIRED_DURATION_TOPIC = "data/required_duration";
const char *REQUIRED_SNOOZE_TIME_TOPIC = "data/required_snooze_time";
const char *CALIB_PRESSURE_MAT_TOPIC = "config/calib_pressure_mat";
const char *CALIB_IMU_TOPIC = "config/calib_imu";
const char *NOTIFICATIONS_SETTINGS_TOPIC = "config/notifications_settings";
const char *SELECT_WIFI_TOPIC = "config/wifi";

const char *CURRENT_BACK_REST_ANGLE_TOPIC = "data/current_back_rest_angle";
const char *CURRENT_PRESSURE_MAT_DATA_TOPIC = "data/current_pressure_mat_data";
const char *CURRENT_IS_SOMEONE_THERE_TOPIC = "data/current_is_someone_there";
const char *CURRENT_IS_WIFI_CONNECTED_TOPIC = "data/current_is_wifi_connected";
const char *CURRENT_CHAIR_SPEED_TOPIC = "data/current_chair_speed";
const char *HEARTBEAT_TOPIC = "heartbeat/embedded";
const char *VIBRATION_TOPIC = "data/vibration";
const char *IS_MOVING_TOPIC = "data/is_moving";
const char *TILT_INFO_TOPIC = "data/tilt_info";

const char *SENSOR_STATUS_TOPIC = "status/sensor";
const char *SENSORS_STATUS_TOPIC = "status/sensors";

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
        subscribe(NULL, REQUIRED_ANGLE_TOPIC);
        subscribe(NULL, REQUIRED_PERIOD_TOPIC);
        subscribe(NULL, REQUIRED_DURATION_TOPIC);
        subscribe(NULL, CALIB_PRESSURE_MAT_TOPIC);
        subscribe(NULL, CALIB_IMU_TOPIC);
        subscribe(NULL, NOTIFICATIONS_SETTINGS_TOPIC);
        subscribe(NULL, SELECT_WIFI_TOPIC);
        subscribe(NULL, REQUIRED_SNOOZE_TIME_TOPIC);
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

    if (topic == ALARM_TOPIC)
    {
        try
        {
            _setAlarmOn = std::stoi(message);
            _setAlarmOnNew = true;
        }
        catch (const std::exception &e)
        {
            printf(EXCEPTION_MESSAGE, e.what());
            printf("Setting _setAlarmOn and _setAlarmOnNew to false\n");
            _setAlarmOn = false;
            _setAlarmOnNew = false;
        }
    }
    else if (topic == SELECT_WIFI_TOPIC)
    {
        _wifiChanged = true;
        _wifiInformation = message;
    }
    else if (topic == REQUIRED_ANGLE_TOPIC)
    {
        try
        {
            _requiredBackRestAngle = std::stoi(message);
            _requiredBackRestAngleNew = true;
        }
        catch (const std::exception &e)
        {
            printf(EXCEPTION_MESSAGE, e.what());
            printf("Setting _requiredBackRestAngle to 0 and _requiredBackRestAngleNew to false\n");
            _requiredBackRestAngle = 0;
            _requiredBackRestAngleNew = false;
        }
    }
    else if (topic == REQUIRED_PERIOD_TOPIC)
    {
        try
        {
            _requiredPeriod = std::stoi(message);
            _requiredPeriodNew = true;
        }
        catch (const std::exception &e)
        {
            printf(EXCEPTION_MESSAGE, e.what());
            printf("Setting _requiredPeriod to 0 and _requiredPeriodNew to false\n");
            _requiredPeriod = 0;
            _requiredPeriodNew = false;
        }
    }
    else if (topic == REQUIRED_DURATION_TOPIC)
    {
        try
        {
            _requiredDuration = std::stoi(message);
            _requiredDurationNew = true;
        }
        catch (const std::exception &e)
        {
            printf(EXCEPTION_MESSAGE, e.what());
            printf("Setting _requiredDuration to 0 and _requiredDurationNew to false\n");
            _requiredDuration = 0;
            _requiredDurationNew = false;
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
        _notificationsSettings = message;
        _isNotificationsSettingsChanged = true;
    }
    else
    {
        throw "Error: Invalid topic";
    }
}

void MosquittoBroker::SendBackRestAngle(const int angle, const std::string datetime)
{
    std::string strAngle = std::to_string(angle);
    std::string strMsg = "{\"datetime\":" + datetime + ",\"angle\":" + strAngle + "}";

    PublishMessage(CURRENT_BACK_REST_ANGLE_TOPIC, strMsg);
}

void MosquittoBroker::SendPressureMatData(const pressure_mat_data_t data, const std::string datetime)
{
    rapidjson::Document document;
    document.SetObject();
    rapidjson::Value array(rapidjson::kArrayType);
    rapidjson::Document::AllocatorType &allocator = document.GetAllocator();
    Value dateTimeString(datetime.c_str(), allocator);
    document.AddMember("datetime", dateTimeString, allocator);

    rapidjson::Value centerObject(rapidjson::kObjectType);
    Value centerOfPressureX(std::to_string(data.centerOfPressure.x).c_str(), allocator);
    centerObject.AddMember("x", centerOfPressureX, allocator);
    Value centerOfPressureY(std::to_string(data.centerOfPressure.y).c_str(), allocator);
    centerObject.AddMember("y", centerOfPressureY, allocator);
    document.AddMember("center", centerObject, allocator);

    Value quadrants(rapidjson::kArrayType);

    Value quadrant1(rapidjson::kObjectType);
    Value quadrant1CenterOfPressureX(std::to_string(data.quadrantPressure[0].x).c_str(), allocator);
    quadrant1.AddMember("x", quadrant1CenterOfPressureX, allocator);
    Value quadrant1CenterOfPressureY(std::to_string(data.quadrantPressure[0].y).c_str(), allocator);
    quadrant1.AddMember("y", quadrant1CenterOfPressureY, allocator);
    quadrants.PushBack(quadrant1, allocator);

    Value quadrant2(rapidjson::kObjectType);
    Value quadrant2CenterOfPressureX(std::to_string(data.quadrantPressure[1].x).c_str(), allocator);
    quadrant2.AddMember("x", quadrant2CenterOfPressureX, allocator);
    Value quadrant2CenterOfPressureY(std::to_string(data.quadrantPressure[1].y).c_str(), allocator);
    quadrant2.AddMember("y", quadrant2CenterOfPressureY, allocator);
    quadrants.PushBack(quadrant2, allocator);

    Value quadrant3(rapidjson::kObjectType);
    Value quadrant3CenterOfPressureX(std::to_string(data.quadrantPressure[2].x).c_str(), allocator);
    quadrant3.AddMember("x", quadrant3CenterOfPressureX, allocator);
    Value quadrant3CenterOfPressureY(std::to_string(data.quadrantPressure[2].y).c_str(), allocator);
    quadrant3.AddMember("y", quadrant3CenterOfPressureY, allocator);
    quadrants.PushBack(quadrant3, allocator);

    Value quadrant4(rapidjson::kObjectType);
    Value quadrant4CenterOfPressureX(std::to_string(data.quadrantPressure[3].x).c_str(), allocator);
    quadrant4.AddMember("x", quadrant4CenterOfPressureX, allocator);
    Value quadrant4CenterOfPressureY(std::to_string(data.quadrantPressure[3].y).c_str(), allocator);
    quadrant4.AddMember("y", quadrant4CenterOfPressureY, allocator);
    quadrants.PushBack(quadrant4, allocator);

    document.AddMember("quadrants", quadrants, allocator);

    StringBuffer strMsg;
    Writer<StringBuffer> writer(strMsg);
    document.Accept(writer);

    PublishMessage(CURRENT_PRESSURE_MAT_DATA_TOPIC, strMsg.GetString());
}

void MosquittoBroker::SendIsSomeoneThere(const bool state, const std::string datetime)
{
    std::string strState = std::to_string(state);
    std::string strMsg = "{\"datetime\":" + datetime + ",\"IsSomeoneThere\":" + strState + "}";

    PublishMessage(CURRENT_IS_SOMEONE_THERE_TOPIC, strMsg);
}

void MosquittoBroker::SendIsWifiConnected(const bool state, const std::string datetime)
{
    std::string strState = std::to_string(state);
    std::string strMsg = "{\"datetime\":" + datetime + ",\"IsWifiConnected\":" + strState + "}";

    PublishMessage(CURRENT_IS_WIFI_CONNECTED_TOPIC, strMsg);
}

void MosquittoBroker::SendIsPressureMatCalib(const bool state, const std::string datetime)
{
    std::string strState = std::to_string(state);
    std::string strMsg = "{\"datetime\":" + datetime + ",\"IsPressureMatCalib\":" + strState + "}";

    PublishMessage(CALIB_PRESSURE_MAT_TOPIC, strMsg);
}

void MosquittoBroker::SendIsIMUCalib(const bool state, const std::string datetime)
{
    std::string strState = std::to_string(state);
    std::string strMsg = "{\"datetime\":" + datetime + ",\"IsIMUCalib\":" + strState + "}";

    PublishMessage(CALIB_IMU_TOPIC, strMsg);
}

void MosquittoBroker::SendSpeed(const float speed, const std::string datetime)
{
    std::string strSpeed = std::to_string(speed);
    std::string strMsg = "{\"datetime\":" + datetime + ",\"vitesse\":" + strSpeed + "}";

    PublishMessage(CURRENT_CHAIR_SPEED_TOPIC, strMsg);
}

void MosquittoBroker::SendVibration(double acceleration, const std::string datetime)
{
    std::string strAcceleration = std::to_string(acceleration);
    std::string strMsg = "{\"datetime\":" + datetime + ",\"vibration\":" + strAcceleration + "}";

    PublishMessage(VIBRATION_TOPIC, strMsg);
}

void MosquittoBroker::SendIsMoving(const bool state, const std::string datetime)
{
    std::string strState = std::to_string(state);
    std::string strMsg = "{\"datetime\":" + datetime + ",\"isMoving\":" + strState + "}";

    PublishMessage(IS_MOVING_TOPIC, strMsg);
}

void MosquittoBroker::SendTiltInfo(const int info, const std::string datetime)
{
    std::string strInfo = std::to_string(info);
    std::string strMsg = "{\"datetime\":" + datetime + ",\"info\":" + strInfo + "}";

    PublishMessage(TILT_INFO_TOPIC, strMsg);
}

void MosquittoBroker::SendHeartbeat(const std::string datetime)
{
    std::string strMsg = "{\"datetime\":" + datetime + "}";

    PublishMessage(HEARTBEAT_TOPIC, strMsg);
}

void MosquittoBroker::SendSensorsState(const bool alarmStatus, const bool mobileImuStatus,
                                       const bool fixedImuStatus, const bool motionSensorStatus, const bool plateSensorStatus,
                                       const std::string datetime)
{
    std::string strMsg = "{"
        "\"datetime\":" +  datetime + ","
        "\"alarm\":" + std::to_string(alarmStatus) + ","
        "\"mobileImu\":" + std::to_string(mobileImuStatus) + ","
        "\"fixedImu\":" + std::to_string(fixedImuStatus) + ","
        "\"motionSensor\":" + std::to_string(motionSensorStatus) + ","
        "\"forcePlate\":" + std::to_string(plateSensorStatus) + "}";

    PublishMessage(SENSORS_STATUS_TOPIC, strMsg);
}

void MosquittoBroker::SendSensorState(const int device, const bool status, const std::string datetime)
{
    std::string sensorName;

    switch (device)
    {
        case alarmSensor:
            sensorName = "alarm";
            break;
        case mobileImu:
            sensorName = "mobileImu";
            break;
        case fixedImu:
            sensorName = "fixedImu";
            break;
        case motionSensor:
            sensorName = "motionSensor";
            break;
        case plateSensor:
            sensorName = "plateSensor";
            break;
        default:
            throw "Error: Invalid device";
            break;
    }

    const std::string strState = std::to_string(status);
    const std::string strMsg = "{\"datetime\":" + datetime + ",\"" + sensorName + "\":" + strState + "}";

    PublishMessage(SENSORS_STATUS_TOPIC, strMsg);
}

bool MosquittoBroker::GetSetAlarmOn()
{
    _setAlarmOnNew = false;
    return _setAlarmOn;
}

uint32_t MosquittoBroker::GetRequiredBackRestAngle()
{
    _requiredBackRestAngleNew = false;
    return _requiredBackRestAngle;
}

uint32_t MosquittoBroker::GetRequiredPeriod()
{
    _requiredPeriodNew = false;
    return _requiredPeriod;
}

uint32_t MosquittoBroker::GetRequiredDuration()
{
    _requiredDurationNew = false;
    return _requiredDuration;
}

std::string MosquittoBroker::GetWifiInformation()
{
    _wifiChanged = false;
    return _wifiInformation;
}

std::string MosquittoBroker::GetNotificationsSettings()
{
    _isNotificationsSettingsChanged = false;
    return _notificationsSettings;
}

bool MosquittoBroker::CalibPressureMatRequired()
{
    if (_calibPressureMatRequired)
    {
        _calibPressureMatRequired = false;
        return true;
    }
    else
    {
        return false;
    }
}

bool MosquittoBroker::CalibIMURequired()
{
    if (_calibIMURequired)
    {
        _calibIMURequired = false;
        return true;
    }
    else
    {
        return false;
    }
}

void MosquittoBroker::PublishMessage(const char *topic, const std::string message)
{
    if (message.empty())
    {
        printf("Error: Empty mosquitto message\n");
        return;
    }
    publish(NULL, topic, message.length(), message.c_str());
}
