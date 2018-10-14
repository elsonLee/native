#include "Timer.h"

Timer::Timer (const std::function<void()>& cb, Timestamp expiration, MicroSeconds interval) :
    _callback(cb),
    _expiration(expiration),
    _interval(interval)
{}

Timer::~Timer ()
{}

//Timer::restart (Time)

