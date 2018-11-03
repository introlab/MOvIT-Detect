#pragma once

#include "Process.h"

#include <map>
#include <chrono>

class ProcessControl
{
  public:
    ProcessControl();
    ~ProcessControl() = default;

    void AddProcess(ProcessType type, Process *proc);
    void StartAll();
    void StopAll();

    void CheckProcesses();

    void UpdateHeartbeat(ProcessType type);

  private:
    static constexpr auto STALE_PROCESS_TIME = std::chrono::milliseconds(5000);

    std::map<ProcessType, Process*> _processes;
    std::map<ProcessType, std::chrono::time_point<std::chrono::system_clock>> _processTimestamp;

    void RestartProcess(Process *process);
};
