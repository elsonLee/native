#pragma once

#include <iomanip>
#include <ctime>
#include <chrono>

using Clock = std::chrono::high_resolution_clock;
using Timestamp = Clock::time_point;
using Duration = Clock::duration;
using MicroSeconds = std::chrono::microseconds;
