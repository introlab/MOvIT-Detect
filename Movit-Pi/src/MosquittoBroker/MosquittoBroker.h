#ifndef MOSQUITTO_BROKER_H
#define MOSQUITTO_BROKER_H

#include <mosquittopp.h>
#include <stdint.h>
#include <string>

// Pour reference
// struct mosquitto_message
// {
// 	int mid;
// 	char *topic;
// 	void *payload;
// 	int payloadlen;
// 	int qos;
// 	bool retain;
// };

class MosquittoBroker : public mosqpp::mosquittopp
{
  public:
	MosquittoBroker(const char *id);
	~MosquittoBroker();

	void on_connect(int rc);
	void on_publish(int mid);
	void on_subcribe(int mid, int qos_count, const int *granted_qos);
	void on_message(const mosquitto_message *message);

	void sendBackRestAngle(const int angle, const std::string datetime);
	void sendCenterOfPressure(const float x, const float y, const std::string datetime);
	void sendIsSomeoneThere(const bool state, const std::string datetime);
	void sendSpeed(const float speed, const std::string datetime);

	bool getSetAlarmOn();
	uint32_t getRequiredBackRestAngle();
	uint32_t getRequiredPeriod();
	uint32_t getRequiredDuration();

	bool isSetAlarmOnNew() { return _setAlarmOnNew; }
	bool isRequiredBackRestAngleNew() { return _requiredBackRestAngleNew; }
	bool isRequiredPeriodNew() { return _requiredPeriodNew; }
	bool isRequiredDurationNew() { return _requiredDurationNew; }

  private:
	bool _setAlarmOn = false;
	uint32_t _requiredBackRestAngle = 0;
	uint32_t _requiredPeriod = 0;
	uint32_t _requiredDuration = 0;

	bool _setAlarmOnNew = false;
	bool _requiredBackRestAngleNew = false;
	bool _requiredPeriodNew = false;
	bool _requiredDurationNew = false;
};

#endif
