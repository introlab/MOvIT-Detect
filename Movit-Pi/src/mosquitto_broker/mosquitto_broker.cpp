#include <iostream>
#include <cstdio>
#include <string>

#include "mosquitto_broker.h"
#include <mosquittopp.h>

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
	// printf("Subscriber received message mid: %i of topic: %s Data: %s\n", msg->mid, msg->topic, reinterpret_cast<char *>(msg->payload));

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
			printf("Exception thrown by %s()\n", e.what());
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
			printf("Exception thrown by %s()\n", e.what());
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
			printf("Exception thrown by %s()\n", e.what());
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
			printf("Exception thrown by %s()\n", e.what());
			printf("Setting _requiredDuration to 0\n");
			_requiredDuration = 0;
			_requiredDurationNew = false;
		}
	}
}

void MosquittoBroker::sendBackRestAngle(const int angle, const std::string datetime)
{
	std::string strAngle = std::to_string(angle);
	std::string strMsg = "{\"datetime\":" + datetime + ",\"angle\":" + strAngle + "}";
	publish(NULL, "data/current_back_rest_angle", strMsg.length(), strMsg.c_str());
}

void MosquittoBroker::sendCenterOfPressure(const float x, const float y, const std::string datetime)
{
	std::string strMsg = "{\"datetime\":" + datetime + ",\"pos_x\":" + std::to_string(x) + ",\"pos_y\":" + std::to_string(x) + "}";

	publish(NULL, "data/current_center_of_pressure", strMsg.length(), strMsg.c_str());
}

void MosquittoBroker::sendIsSomeoneThere(const bool state, const std::string datetime)
{
	std::string strState = std::to_string(state);
	std::string strMsg = "{\"datetime\":" + datetime + ",\"IsSomeoneThere\":" + strState + "}";

	publish(NULL, "data/current_is_someone_there", strMsg.length(), strMsg.c_str());
}

void MosquittoBroker::sendSpeed(const float speed, const std::string datetime)
{
	std::string strSpeed = std::to_string(speed);
	std::string strMsg = "{\"datetime\":" + datetime + ",\"vitesse\":" + strSpeed + "}";

	publish(NULL, "data/current_chair_speed", strMsg.length(), strMsg.c_str());
}

bool MosquittoBroker::getSetAlarmOn()
{
	_setAlarmOnNew = false;
	return _setAlarmOn;
}

uint32_t MosquittoBroker::getRequiredBackRestAngle()
{
	_requiredBackRestAngleNew = false;
	return _requiredBackRestAngle;
}

uint32_t MosquittoBroker::getRequiredPeriod()
{
	_requiredPeriodNew = false;
	return _requiredPeriod;
}

uint32_t MosquittoBroker::getRequiredDuration()
{
	_requiredDurationNew = false;
	return _requiredDuration;
}