#ifndef MOSQUITTO_BROKER_H
#define MOSQUITTO_BROKER_H

#include "mosquittopp.h"
#include "Utils.h"
#include "DataType.h"
#include <stdint.h>
#include <string>
#include "SeatingFSM.h"
#include "TravelFSM.h"
#include "AngleFSM.h"
#include "NotificationFSM.h"

class MosquittoBroker : public mosqpp::mosquittopp
{
  public:
    MosquittoBroker(const char *id);
    ~MosquittoBroker();

    void SendHeartbeat(const std::string datetime);

    void SendSensorsData(SensorData sd);
    void SendChairState(ChairState cs);
    void SendAngleFSM(AngleFSM angleFSM);
    void SendTravelFSM(TravelFSM travelFSM);
    void SendSeatingFSM(SeatingFSM seatingFSM);
    void SendNotificationFSM(NotificationFSM nFSM);
    void SendIsWifiConnected(const bool state, const std::string datetime);
    void SendAngleOffset(int mIMUOffset, int fIMUOffset);

    bool CalibIMURequired();
    void SetCalibIMURequired(bool required);

    bool CalibPressureMatRequired();
    void SetCalibPressurMatRequired(bool required);


    bool GoalHasChanged();
    void SetGoalHasChanged(bool changed);
    void getGoal(int *frequency, int *duration, int *angle);
    bool NotificationHasChanged();
    void SetNotificationHasChanged(bool changed);
    void getNotificationSettings(bool *ledBlinkingEnable, bool *vibrationEnabled, int *snoozeTime, bool *isEnabled);
    bool offsetChanged();
    void setOffsetChanged(bool changed);
    void getOffsets(int *mIMUOffset, int *fIMUOffset);
    
    //DL Nov 26 2020 - new flags
    bool getAlarmEnabled() {return _alarmEnabled;}
    bool getAlarmAlternatingLedBlink() {return _alarmAlternatingLedBlink;}
    bool getAlarmMotorOn() {return _alarmMotorOn;}
    bool getAlarmRedLedOn() {return _alarmRedLedOn;}
    bool getAlarmGreenLedOn() {return _alarmGreenLedOn;}
    bool getAlarmGreenLedBlink() {return _alarmGreenLedBlink;}
    

  private:
    void on_connect(int rc);
    void on_publish(int mid);
    void on_subcribe(int mid, int qos_count, const int *granted_qos);
    void on_message(const mosquitto_message *message);

    void PublishMessage(const char *topic, const std::string message, int qos = 0, bool retain = false);
    void splitStringWithDelemiter(std::string toSplit, std::string delimiter, std::string *elements, int *numElem);

    bool _setAlarmOn = false;
    tilt_settings_t _tiltSettings;

    std::string _wifiInformation = "";

    int _frequency = 1000;
    int _duration = 1000;
    int _angle = 30;
    bool _shouldBlink = 1;
    bool _shouldVibrate = 0;
    bool _isEnabled = 1;
    int _snoozeTime = 60;
    int _mIMUOffset = 0;
    int _fIMUOffset = 0;
    bool _OffsetChanged = false;

    bool _isNotificationsSettingsChanged = false;
    bool _calibPressureMatRequired = false;
    bool _isTiltSettingsChanged = false;
    bool _calibIMURequired = false;
    bool _setAlarmOnNew = false;
    bool _wifiChanged = false;
    bool _GoalChanged = false;
    
    //DL Nov 26 2020 - new flags
    bool _alarmEnabled = false;
    bool _alarmAlternatingLedBlink = false;
    bool _alarmMotorOn = false;
    bool _alarmRedLedOn = false;
    bool _alarmGreenLedOn = false;
    bool _alarmGreenLedBlink = false;
    
};

#endif // MOSQUITTO_BROKER_H
