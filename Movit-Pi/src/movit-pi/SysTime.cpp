#include "SysTime.h"
#include <thread>
#include <iostream>

void sleep_for_microseconds(uint32_t microseconds)
{
    std::this_thread::sleep_for(std::chrono::microseconds(microseconds));
}

void sleep_for_milliseconds(uint32_t milliseconds)
{
    //std::cout<<"sleep_for_milliseconds: " << milliseconds <<std::endl;
    std::this_thread::sleep_for(std::chrono::milliseconds(milliseconds));
}

void sleep_for_seconds(uint32_t seconds)
{
    std::this_thread::sleep_for(std::chrono::seconds(seconds));
}
