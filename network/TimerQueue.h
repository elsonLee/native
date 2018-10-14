#pragma once

#include <functional>
#include <chrono>
#include <utility>
#include <set>

#include "Utility.h"
#include "Channel.h"
#include "Timestamp.h"

class Timer;

class TimerQueue
{
    public:
        TimerQueue (EventLoop* loop);
        ~TimerQueue ();

        TimerQueue (const TimerQueue&) = delete;
        TimerQueue& operator= (const TimerQueue&) = delete;

        void addTimer (const std::function<void()>& cb, Timestamp expiration, MicroSeconds interval);

        //void cancel (int timerId);

    private:
        bool insert (Timer* timer);
        void addTimerInLoop (Timer* timer);
        void handleRead ();
        std::vector<std::pair<Timestamp, Timer*>> getExpired (Timestamp now);

    private:
        EventLoop*      _loop;
        FileDescriptor  _fd;
        Channel         _timersChannel;
        std::set<std::pair<Timestamp, Timer*>> _timers;
};
