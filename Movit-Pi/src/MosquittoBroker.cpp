#include <iostream>
#include <cstdio>
#include <string>

#include "MosquittoBroker.h"
#include "mosquittopp.h"

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
const char *CALIB_PRESSURE_MAT_TOPIC = "config/calib_pressure_mat";
const char *CALIB_IMU_TOPIC = "config/calib_imu";
const char *DEACTIVATE_VIBRATION = "config/deactivate_vibration";

const char *CURRENT_BACK_REST_ANGLE_TOPIC = "data/current_back_rest_angle";
const char *CURRENT_CENTER_OF_PRESSURE_TOPIC = "data/current_center_of_pressure";
const char *CURRENT_IS_SOMEONE_THERE_TOPIC = "data/current_is_someone_there";
const char *CURRENT_CHAIR_SPEED_TOPIC = "data/current_chair_speed";
const char *KEEP_ALIVE = "data/keep_alive";
const char *VIBRATION_TOPIC = "data/vibration";

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
        subscribe(NULL, DEACTIVATE_VIBRATION);
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
    std::string message(reinterpret_cast<char *>(msg->payload));
    std::string topic(msg->topic);

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
            printf("Setting _setAlarmOn to false\n");
            _setAlarmOn = false;
            _setAlarmOnNew = false;
        }
    }
    if (topic == REQUIRED_ANGLE_TOPIC)
    {
        try
        {
            _requiredBackRestAngle = std::stoi(message);
            _requiredBackRestAngleNew = true;
        }
        catch (const std::exception &e)
        {
            printf(EXCEPTION_MESSAGE, e.what());
            printf("Setting _requiredBackRestAngle to 0\n");
            _requiredBackRestAngle = 0;
            _requiredBackRestAngleNew = false;
        }
    }
    if (topic == REQUIRED_PERIOD_TOPIC)
    {
        try
        {
            _requiredPeriod = std::stoi(message);
            _requiredPeriodNew = true;
        }
        catch (const std::exception &e)
        {
            printf(EXCEPTION_MESSAGE, e.what());
            printf("Setting _requiredPeriod to 0\n");
            _requiredPeriod = 0;
            _requiredPeriodNew = false;
        }
    }
    if (topic == REQUIRED_DURATION_TOPIC)
    {
        try
        {
            _requiredDuration = std::stoi(message);
            _requiredDurationNew = true;
        }
        catch (const std::exception &e)
        {
            printf(EXCEPTION_MESSAGE, e.what());
            printf("Setting _requiredDuration to 0\n");
            _requiredDuration = 0;
            _requiredDurationNew = false;
        }
    }
    if (topic == CALIB_PRESSURE_MAT_TOPIC)
    {
        try
        {
            _calibPressureMatRequired = std::stoi(message);
        }
        catch (const std::exception &e)
        {
            printf(EXCEPTION_MESSAGE, e.what());
            printf("Setting _requiredDuration to 0\n");
            _calibPressureMatRequired = false;
        }
    }
    if (topic == CALIB_IMU_TOPIC)
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
    if (topic == DEACTIVATE_VIBRATION)
    {
        try
        {
            printf("Something new for _isVibrationDeactivated = %s\n", message.c_str());
            _isVibrationDeactivated = std::stoi(message);
        }
        catch (const std::exception &e)
        {
            printf(EXCEPTION_MESSAGE, e.what());
            printf("Setting _deactivate_vibration to 0\n");
            _isVibrationDeactivated = false;
        }
    }
}

void MosquittoBroker::SendBackRestAngle(const int angle, const std::string datetime)
{
    std::string strAngle = std::to_string(angle);
    std::string strMsg = "{\"datetime\":" + datetime + ",\"angle\":" + strAngle + "}";
    publish(NULL, CURRENT_BACK_REST_ANGLE_TOPIC, strMsg.length(), strMsg.c_str());
}

void MosquittoBroker::SendCenterOfPressure(const float x, const float y, const std::string datetime)
{
    std::string strMsg = "{\"datetime\":" + datetime + ",\"pos_x\":" + std::to_string(x) + ",\"pos_y\":" + std::to_string(y) + "}";

    publish(NULL, CURRENT_CENTER_OF_PRESSURE_TOPIC, strMsg.length(), strMsg.c_str());
}

void MosquittoBroker::SendIsSomeoneThere(const bool state, const std::string datetime)
{
    std::string strState = std::to_string(state);
    std::string strMsg = "{\"datetime\":" + datetime + ",\"IsSomeoneThere\":" + strState + "}";

    publish(NULL, CURRENT_IS_SOMEONE_THERE_TOPIC, strMsg.length(), strMsg.c_str());
}

void MosquittoBroker::SendIsPressureMatCalib(const bool state, const std::string datetime)
{
    std::string strState = std::to_string(state);
    std::string strMsg = "{\"datetime\":" + datetime + ",\"IsPressureMatCalib\":" + strState + "}";

    publish(NULL, CALIB_PRESSURE_MAT_TOPIC, strMsg.length(), strMsg.c_str());
}

void MosquittoBroker::SendIsIMUCalib(const bool state, const std::string datetime)
{
    std::string strState = std::to_string(state);
    std::string strMsg = "{\"datetime\":" + datetime + ",\"IsIMUCalib\":" + strState + "}";

    publish(NULL, CALIB_IMU_TOPIC, strMsg.length(), strMsg.c_str());
}

void MosquittoBroker::SendSpeed(const float speed, const std::string datetime)
{
    std::string strSpeed = std::to_string(speed);
    std::string strMsg = "{\"datetime\":" + datetime + ",\"vitesse\":" + strSpeed + "}";

    publish(NULL, CURRENT_CHAIR_SPEED_TOPIC, strMsg.length(), strMsg.c_str());
}

void MosquittoBroker::SendKeepAlive(const std::string datetime)
{
    std::string strMsg = "{\"datetime\":" + datetime + "}";

    publish(NULL, KEEP_ALIVE, strMsg.length(), strMsg.c_str());
}

void MosquittoBroker::SendVibration(double acceleration, const std::string datetime)
{
    std::string strAcceleration = std::to_string(acceleration);
    std::string strMsg = "{\"datetime\":" + datetime + ",\"vibration\":" + strAcceleration + "}";

    publish(NULL, VIBRATION_TOPIC, strMsg.length(), strMsg.c_str());
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
