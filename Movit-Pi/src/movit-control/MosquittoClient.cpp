#include "MosquittoClient.h"
#include <cstdio>
#include <string>

using std::string;

MosquittoClient::MosquittoClient(const char *id) : mosquittopp(id)
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

MosquittoClient::~MosquittoClient()
{
    disconnect();

    if (loop_stop() != MOSQ_ERR_SUCCESS)
    {
        printf("Failed to stop the network thread previously created.\n");
    }

    mosqpp::lib_cleanup();
}

void MosquittoClient::on_connect(int rc)
{
    printf("CONTROL Connected with code %d.\n", rc);
    if (rc == 0)
    {
        subscribe(NULL, EMBEDDED_HEARTBEAT_TOPIC);
        subscribe(NULL, BACKEND_HEARTBEAT_TOPIC);
    }
}

void MosquittoClient::on_publish(int mid)
{
}

void MosquittoClient::on_message(const mosquitto_message *msg)
{
    string message;
    string topic;

    if (msg->payload != NULL)
    {
        message = reinterpret_cast<char *>(msg->payload);
    }
    else
    {
        message = "";
    }
    topic = msg->topic;

    if (topic == EMBEDDED_HEARTBEAT_TOPIC)
    {
        // On call le callback du process embarqué si présent
        _callback(ProcessType::EMBEDDED);
    }

    if (topic == BACKEND_HEARTBEAT_TOPIC)
    {
        // On call le callback du process backend
        _callback(ProcessType::BACKEND);
    }
}
