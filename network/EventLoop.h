#pragma once

#include "Timestamp.h"
#include "Channel.h"

#include <cassert>
#include <memory>
#include <iostream>
#include <thread>
#include <functional>
#include <mutex>
#include <vector>
#include <atomic>

class Channel;
class Poller;
class TimerQueue;

//! NOTE:
//  EventLoop corresponds to the Initiation Dispatcher in Reactor model,
//  with updateChannel & removeChannel APIs to register/unregister the
//  event handlers
//  Poller is Synchronous Event Demultiplexer
//  Channel is Event Handler

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

        //! thread-safe
        void runInLoop (const std::function<void()>& cb);

        void queueInLoop (const std::function<void()>& cb);

        bool updateChannel (const Channel* ch);

        bool removeChannel (const Channel* ch);

        bool isRunning () const { return _is_running.load(); }

        void quit ();

        //! thread-safe
        void runAt (const Timestamp& time, const std::function<void()>& cb);

        void runAfter (int us, const std::function<void()>& cb);

        void runEvery (int intervalInMicroSeconds, const std::function<void()>& cb);

    private:

        void pushCallback (const std::function<void()>& cb);

        //std::function<void()> popCallback (void);
        //
        void handlePendingCallback (void);

        void wakeup (void);

    private:

        std::atomic<bool>                   _is_running;

        std::atomic<bool>                   _quit;

        std::thread::id                     _thread_id;

        bool                                _handling_pending_callback;

        std::unique_ptr<Poller>             _poller;

        std::unique_ptr<TimerQueue>         _timer_queue;

        int                                 _wakeup_fd;

        std::unique_ptr<Channel>            _wakeup_channel;

        std::mutex                          _func_queue_mtx;

        std::vector<std::function<void()>>  _func_queue;
};
