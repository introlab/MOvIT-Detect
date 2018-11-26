#ifndef MOSQUITTO_BROKER_H
#define MOSQUITTO_BROKER_H

#include "mosquittopp.h"
#include "Utils.h"
#include "DataType.h"
#include <stdint.h>
#include <string>

class MosquittoBroker : public mosqpp::mosquittopp
{
  public:
    MosquittoBroker(const char *id);
    ~MosquittoBroker();

    void on_connect(int rc);
    void on_publish(int mid);
    void on_subcribe(int mid, int qos_count, const int *granted_qos);
    void on_message(const mosquitto_message *message);

    void SendBackRestAngle(const int angle, const std::string datetime);
    void SendPressureMatData(const pressure_mat_data_t data, const std::string datetime);
    void SendIsSomeoneThere(const bool state, const std::string datetime);
    void SendIsPressureMatCalib(const bool state, const std::string datetime);
    void SendIsIMUCalib(const bool state, const std::string datetime);
    void SendSpeed(const float speed, const std::string datetime);
    void SendHeartbeat(const std::string datetime);
    void SendVibration(double acceleration, const std::string datetime);
    void SendIsMoving(const bool state, const std::string datetime);
    void SendTiltInfo(const int info, const std::string datetime);

    void SendSensorsState(sensor_state_t sensorState, const std::string datetime);
    void SendIsWifiConnected(const bool state, const std::string datetime);

    bool GetSetAlarmOn();
    tilt_settings_t GetTiltSettings();
    std::string GetWifiInformation();
    float GetSnoozeTime();

    bool IsSetAlarmOnNew() { return _setAlarmOnNew; }
    bool IsTiltSettingsChanged() { return _isTiltSettingsChanged; }
    bool IsWifiChanged() { return _wifiChanged; }

    bool CalibPressureMatRequired();
    bool CalibIMURequired();

    bool IsNotificationsSettingsChanged() { return _isNotificationsSettingsChanged; }
    notifications_settings_t GetNotificationsSettings();

  private:
    void PublishMessage(const char *topic, const std::string message);

    bool _setAlarmOn = false;
    tilt_settings_t _tiltSettings;

    std::string _wifiInformation = "";
    notifications_settings_t _notificationsSettings;

    bool _isNotificationsSettingsChanged = false;
    bool _calibPressureMatRequired = false;
    bool _isTiltSettingsChanged = false;
    bool _calibIMURequired = false;
    bool _setAlarmOnNew = false;
    bool _wifiChanged = false;
};

#endif // MOSQUITTO_BROKER_H
