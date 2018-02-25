#ifndef MOSQUITTO_BROKER_H
#define MOSQUITTO_BROKER_H

#include <mosquittopp.h>

class MosquittoBroker : public mosqpp::mosquittopp
{
	public:
		MosquittoBroker(const char *id);
		~MosquittoBroker();

		void on_connect(int rc);
		void on_publish(int mid);

		void sendBackRestAngle(const int angle);
		void sendSittingTime(const int time);
		void sendDistanceTraveled(const int distance);
};

#endif
