#include "Timer.h"
#include "Timestamp.h"
#include "EventLoop.h"

#include <sys/timerfd.h>
#include <unistd.h>
#include <strings.h>
#include <iostream>
#include <vector>

#include "TimerQueue.h"

static int
createTimerFd ()
{
    int fd = ::timerfd_create(CLOCK_MONOTONIC, TFD_NONBLOCK | TFD_CLOEXEC);
    if (fd < 0) {
        std::cerr << "timerfd_create failed!" << std::endl;
    }
    return fd;
}

//! read fd for level trigger completion
static void
readTimerFd (int fd)
{
    uint64_t count;
    ::read(fd, &count, sizeof(count));
}

struct timespec
howMuchTimeFromNow (Timestamp when)
{
    Duration duration = when.time_since_epoch() - Clock::now().time_since_epoch();
    long us = std::chrono::duration_cast<MicroSeconds>(duration).count();
    if (us < 100) {
        us = 100;
    }
    struct timespec ts;
    ts.tv_sec = static_cast<time_t>(us / 1000000);
    ts.tv_nsec = static_cast<long>(us % 1000000) * 1000;
    //std::cout << "us: " << us << " howMuchTimeFromNow: " << ts.tv_sec << "s, " << ts.tv_nsec << "ns" << std::endl;
    return ts;
}

void
resetTimerFd (int timerFd, Timestamp expiration)
{
    struct itimerspec newValue;
    struct itimerspec oldValue;
    bzero(&newValue, sizeof(newValue));
    bzero(&oldValue, sizeof(oldValue));
    newValue.it_value = howMuchTimeFromNow(expiration);
    int ret = ::timerfd_settime(timerFd, 0, &newValue, &oldValue);
    if (ret) {
        std::cerr << "timerfd_settime failed" << std::endl;
    }
}


TimerQueue::TimerQueue (EventLoop* loop) :
    _loop(loop),
    _fd(createTimerFd()),
    _timersChannel("timerqueue", _fd.fd(), loop)
{
    _timersChannel.setReadCallback([this]{ handleRead(); });
    _timersChannel.enableReadEvent();
}

TimerQueue::~TimerQueue ()
{
    _timersChannel.disableAllEvent();
}

void
TimerQueue::addTimer (const std::function<void()>& cb, Timestamp expiration, MicroSeconds interval)
{
    Timer* timer = new Timer(cb, expiration, interval);
    _loop->runInLoop([this, timer]{ addTimerInLoop(timer); });
}

std::vector<std::pair<Timestamp, Timer*>>
TimerQueue::getExpired (Timestamp now)
{
    std::vector<std::pair<Timestamp, Timer*>> expired;
    auto it = _timers.lower_bound(std::make_pair(now, reinterpret_cast<Timer*>(UINTPTR_MAX)));
    std::copy(_timers.begin(), it, std::back_inserter(expired));
    _timers.erase(_timers.begin(), it);
    std::cout << "getExpired: " << expired.size() << std::endl;
    return expired;
}

void
TimerQueue::handleRead ()
{
    readTimerFd(_fd.fd());
    std::vector<std::pair<Timestamp, Timer*>> expired = getExpired(Clock::now());

    for (auto p : expired) {
        p.second->run();
    }

    for (auto p : expired) {
        Timer* timer = p.second;
        if (timer->interval().count() > 0) {
            timer->restart();
            addTimerInLoop(timer);
        } else {
            delete timer;
        }
    }
}

bool
TimerQueue::insert (Timer* timer)
{
    bool earliestChanged = false;
    Timestamp when = timer->expiration();

    auto it = _timers.begin();
    if (it == _timers.end() || when < it->first) {
        earliestChanged = true;
    }

    _timers.emplace(when, timer);

    return earliestChanged;
}

void
TimerQueue::addTimerInLoop (Timer* timer)
{
    bool earliestChanged = insert(timer);
    if (earliestChanged) {
        resetTimerFd(_fd.fd(), timer->expiration());
    }
}
