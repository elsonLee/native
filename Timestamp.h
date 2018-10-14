#ifndef TIMESTAME_H
#define TIMESTAME_H

#include <iomanip>
#include <ctime>
#include <chrono>

using Clock = std::chrono::high_resolution_clock;
using Timestamp = Clock::time_point;
using Duration = Clock::duration;
using MicroSeconds = std::chrono::microseconds;

#endif
