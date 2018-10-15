#pragma once

#include <atomic>

class EventLoop;
class EventLoopThread
{
    public:
        EventLoopThread () = default;
        ~EventLoopThread () = default;

        EventLoopThread (const EventLoopThread&) = delete;
        EventLoopThread& operator= (const EventLoopThread&) = delete;

        EventLoop* start ();
        EventLoop* getLoop () const { return _loop.load(); }

    private:
        std::atomic<EventLoop*>     _loop{nullptr};
};
