#pragma once

#include <stdint.h>

const int SECONDS_TO_MICROSECONDS = 1000000;
const int SECONDS_TO_MILLISECONDS = 1000;

void sleep_for_microseconds(uint32_t microseconds);
void sleep_for_milliseconds(uint32_t milliseconds);
void sleep_for_seconds(uint32_t seconds);

const double RUNNING_FREQUENCY = 10.0f; // Hz
