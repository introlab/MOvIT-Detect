#ifndef MOSQUITTO_BROKER_H
#define MOSQUITTO_BROKER_H

#include "mosquittopp.h"
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
    void SendCenterOfPressure(const float x, const float y, const std::string datetime);
    void SendIsSomeoneThere(const bool state, const std::string datetime);
    void SendIsPressureMatCalib(const bool state, const std::string datetime);
    void SendIsIMUCalib(const bool state, const std::string datetime);
    void SendSpeed(const float speed, const std::string datetime);
    void SendKeepAlive(const std::string datetime);
    void SendVibration(double acceleration, const std::string datetime);

    bool GetSetAlarmOn();
    uint32_t GetRequiredBackRestAngle();
    uint32_t GetRequiredPeriod();
    uint32_t GetRequiredDuration();

    bool IsSetAlarmOnNew() { return _setAlarmOnNew; }
    bool IsRequiredBackRestAngleNew() { return _requiredBackRestAngleNew; }
    bool IsRequiredPeriodNew() { return _requiredPeriodNew; }
    bool IsRequiredDurationNew() { return _requiredDurationNew; }

    bool CalibPressureMatRequired();
    bool CalibIMURequired();

  private:
    bool _setAlarmOn = false;
    uint32_t _requiredBackRestAngle = 0;
    uint32_t _requiredPeriod = 0;
    uint32_t _requiredDuration = 0;

    bool _calibPressureMatRequired = false;
    bool _calibIMURequired = false;

    bool _setAlarmOnNew = false;
    bool _requiredBackRestAngleNew = false;
    bool _requiredPeriodNew = false;
    bool _requiredDurationNew = false;
};

#endif // MOSQUITTO_BROKER_H
