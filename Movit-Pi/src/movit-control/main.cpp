#include <iostream>
#include "ProcessControl.h"
#include "MosquittoClient.h"

#include <chrono>
#include <thread>

using namespace std;

int main(int argc, char *argv[])
{
    MosquittoClient client("control");
    Process procEmbedded("movit-pi", "/home/pi/MOvIT-Detect/Movit-Pi/Executables");
    Process procBackend("sh", "/bin");
    ProcessControl proc_ctrl;

    proc_ctrl.AddProcess(ProcessType::EMBEDDED, &procEmbedded);
    proc_ctrl.AddProcess(ProcessType::BACKEND, &procBackend);

    procBackend.AddArgument("/home/pi/MOvIT-Detect/start_backend.sh");

    // Set le callback qui sera callé quand un heartbeat sera recu
    client.SetCallback(std::bind(&ProcessControl::UpdateHeartbeat, std::ref(proc_ctrl), std::placeholders::_1));

    proc_ctrl.StartAll();

    bool done = false;

    while (!done)
    {
        proc_ctrl.CheckProcesses();
        std::this_thread::sleep_for(std::chrono::milliseconds(1000));
    }

    proc_ctrl.StopAll();

    return 0;
}