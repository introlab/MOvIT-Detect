#include <iostream>
#include <cstdio>
#include <cstring>

#include "mosquitto_broker.h"
#include <mosquittopp.h>

MosquittoBroker::MosquittoBroker(const char *id) : mosquittopp(id)
{
	mosqpp::lib_init();

	const char *username = "admin";
	const char *password = "movitplus";

	const char * host = "localhost";
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
}

void MosquittoBroker::on_publish(int mid)
{
	printf("Message published with id %d.\n", mid);
}

void MosquittoBroker::sendBackRestAngle(const int angle)
{
	std::string strAngle = std::to_string(angle);

	publish(NULL, "data/back_rest_angle", strAngle.length(), strAngle.c_str());
}

void MosquittoBroker::sendSittingTime(const int time)
{
	std::string strTime = std::to_string(time);

	publish(NULL, "data/sitting_time", strTime.length(), strTime.c_str());
}

void MosquittoBroker::sendDistanceTraveled(const int distance)
{
	std::string strDistance = std::to_string(distance);

	publish(NULL, "data/distance_traveled", strDistance.length(), strDistance.c_str());
}