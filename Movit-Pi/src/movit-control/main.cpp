#include <iostream>
#include "Process.h"

#include <chrono>
#include <thread>

using namespace std;

int main(int argc, char *argv[])
{
    Process proc("movit-pi", "~");

    // proc.AddArgument("-t");

    proc.Run();
    std::this_thread::sleep_for(std::chrono::milliseconds(5000));
    proc.Kill();

    return 0;
}