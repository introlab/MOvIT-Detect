#include "Process.h"
#include <unistd.h>
#include <cstdlib>
#include <signal.h>

using std::vector;
using std::string;

Process::Process(std::string executable, std::string path) : _executable(executable),
                                                             _path(path)
{
}

Process::~Process()
{
    Kill();
}

void Process::AddArgument(std::string arg)
{
    if (_arguments.size() < (MAX_ARG_COUNT - 1))
    {
        _arguments.push_back(arg);
    }
}

bool Process::Run()
{
    if (!_executable.size())
    {
        return false;
    }

    _pid = fork();

    char *argv[MAX_ARG_COUNT];

    int count = 0;
    argv[count++] = const_cast<char *>(_executable.c_str());
    for (vector<string>::iterator it = _arguments.begin(); it != _arguments.end(); it++)
    {
        argv[count++] = const_cast<char *>(it->c_str());
    }

    argv[count] = nullptr;

    if (_pid == 0)
    {
        execv(argv[0], argv);
        exit(0);
        return false;
    }

    return true;
}

bool Process::Kill()
{
    if (_pid <= 0)
    {
        return false;
    }

    if (::kill(_pid, SIGINT))
    {
        return false;
    }

    _pid = -1;
    return true;
}