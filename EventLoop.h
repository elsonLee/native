#ifndef EVENT_LOOP_H
#define EVENT_LOOP_H

#include "Timestamp.h"
#include "Channel.h"

#include <cassert>
#include <iostream>
#include <thread>
#include <functional>
#include <mutex>
#include <vector>
//#include <queue>
#include <atomic>

class Channel;
class Poller;
class TimerQueue;

class EventLoop
{
    public:
        EventLoop ();
        ~EventLoop ();

        EventLoop (const EventLoop&) = delete;
        EventLoop& operator= (const EventLoop&) = delete;

        void run ();

        void assertInLoopThread ();
        bool isInLoopThread () const { return _thread_id == std::this_thread::get_id(); }
        void runInLoop (const std::function<void()>& cb);
        void queueInLoop (const std::function<void()>& cb);

        bool updateChannel (const Channel* ch);
        bool removeChannel (const Channel* ch);

        bool isRunning () const { return _is_running.load(); }
        void quit ();

        void runAt (const Timestamp& time, const std::function<void()>& cb);
        void runAfter (int us, const std::function<void()>& cb);
        void runEvery (int intervalInMicroSeconds, const std::function<void()>& cb);

    private:
        void pushCallback (const std::function<void()>& cb);
        //std::function<void()> popCallback (void);
        void handlePendingCallback (void);
        void wakeup (void);

    private:
        std::atomic<bool>                   _is_running;
        std::atomic<bool>                   _quit;
        std::thread::id                     _thread_id;
        Poller*                             _poller;
        TimerQueue*                         _timer_queue;

        int                                 _wakeup_fd;
        Channel*                            _wakeup_channel;

        std::mutex                          _func_queue_mtx;
        std::vector<std::function<void()>>  _func_queue;
};

#endif //EVENT_LOOP_H
