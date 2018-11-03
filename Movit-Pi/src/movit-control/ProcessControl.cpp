#include "ProcessControl.h"

using std::chrono::duration_cast;
using std::chrono::milliseconds;
using std::chrono::system_clock;
using std::chrono::time_point;

ProcessControl::ProcessControl()
{
}

void ProcessControl::AddProcess(ProcessType type, Process *proc)
{
    _processes[type] = proc;
    _processTimestamp[type] = system_clock::now();
}

void ProcessControl::StartAll()
{
    for (std::map<ProcessType, Process *>::iterator it = _processes.begin(); it != _processes.end(); it++)
    {
        it->second->Run();
        it->second->SetState(RunningState::STARTING);
    }
}

void ProcessControl::StopAll()
{
    for (std::map<ProcessType, Process *>::iterator it = _processes.begin(); it != _processes.end(); it++)
    {
        it->second->SetState(RunningState::STOPPED);
        it->second->Kill();
    }
}

void ProcessControl::CheckProcesses()
{
    for (std::map<ProcessType, time_point<system_clock>>::iterator it = _processTimestamp.begin(); it != _processTimestamp.end(); it++)
    {
        auto now = system_clock::now();
        auto processtime = it->second;

        if (duration_cast<milliseconds>(now - processtime) > STALE_PROCESS_TIME)
        {
            // On veux pas détecté qu'un process roule pas quand il est en train de démarrer
            // et qu'il ne peut pas envoyer de heartbeat
            if (_processes[it->first]->Started())
            {
                printf("On essaye de redemarrer %d\n", it->first);
                RestartProcess(_processes[it->first]);
            }
        }
    }
}

void ProcessControl::UpdateHeartbeat(ProcessType type)
{
    _processTimestamp[type] = system_clock::now();

    if (!_processes[type]->Started())
    {
        _processes[type]->SetState(RunningState::STARTED);
        printf("%d est parti !\n", type);
    }
}

void ProcessControl::RestartProcess(Process *process)
{
    process->Kill();
    process->SetState(RunningState::STOPPED);

    process->Run();
    process->SetState(RunningState::STARTING);
}
