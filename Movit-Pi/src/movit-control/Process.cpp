#include "Process.h"
#include <unistd.h>
#include <cstdlib>
#include <signal.h>
#include <cerrno>
#include <cstring>

using std::string;
using std::vector;

Process::Process(string executable, std::string path) : _executable(executable),
                                                        _path(path)
{
}

Process::~Process()
{
    Kill();
}

Process &Process::operator=(const Process &other_proc)
{
    _arguments = other_proc._arguments;
    _executable = other_proc._executable;
    _path = other_proc._path;
    _pid = other_proc._pid;

    return *this;
}

void Process::AddArgument(string arg)
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

    string fullPath;

    if (!_path.size())
    {
        fullPath = _executable;
    }
    else
    {
        fullPath = _path + "/" + _executable;
    }

    argv[count++] = const_cast<char *>(fullPath.c_str());
    for (vector<string>::iterator it = _arguments.begin(); it != _arguments.end(); it++)
    {
        argv[count++] = const_cast<char *>(it->c_str());
    }

    argv[count] = nullptr;

    if (_pid == 0)
    {
        int ret = execv(argv[0], argv);
        if (ret == -1)
        {
            printf("ERROR: Cannot create process: %s. %s\n", _executable.c_str(), std::strerror(errno));
        }
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

    if (::kill(_pid, SIGKILL))
    {
        return false;
    }

    _pid = -1;
    return true;
}
