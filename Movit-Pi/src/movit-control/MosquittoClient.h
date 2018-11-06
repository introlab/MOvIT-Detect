#pragma once

#include "mosquittopp.h"
#include "Process.h"
#include <functional>

class MosquittoClient : public mosqpp::mosquittopp
{
  public:
    MosquittoClient(const char *id);
    ~MosquittoClient();

    void on_connect(int rc);
    void on_publish(int mid);
    void on_subcribe(int mid, int qos_count, const int *granted_qos);
    void on_message(const mosquitto_message *message);

    void SetCallback(std::function<void(ProcessType)> callback) { _callback = callback; };

  private:
    const char *EMBEDDED_HEARTBEAT_TOPIC = "heartbeat/embedded";
    const char *BACKEND_HEARTBEAT_TOPIC = "heartbeat/backend";

    std::function<void(ProcessType)> _callback;
};
