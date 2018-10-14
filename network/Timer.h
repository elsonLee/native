#ifndef TIMER_H
#define TIMER_H

#include "Timestamp.h"

#include <functional>

class Timer
{
    public:
        Timer (const std::function<void()>& cb, Timestamp expiration, MicroSeconds interval);
        ~Timer ();

        void run () const { _callback(); }

        void restart () { _expiration += _interval; }
        Timestamp expiration () const { return _expiration; }
        MicroSeconds interval () const { return _interval; }

    private:
        const std::function<void()> _callback;
        Timestamp _expiration;
        MicroSeconds _interval;


};

#endif
