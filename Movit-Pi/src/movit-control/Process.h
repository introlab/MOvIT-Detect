#pragma once

#include <cstdint>
#include <string>
#include <vector>

class Process
{
  public:
    Process() = default;
    Process(std::string executable, std::string path);
    ~Process();

    void SetExecutable(std::string executable) { _executable = executable; }
    void SetPath(std::string path) { _path = path; }
    void AddArgument(std::string arg);

    bool Run();
    bool Kill();

  private:

    const uint32_t MAX_ARG_COUNT = 32;

    pid_t _pid = -1;                     // pid du processus
    std::string _executable;             // Nom de l'executable
    std::string _path;                   // Path de l'executable
    std::vector<std::string> _arguments; // Arguments de l'executable
};