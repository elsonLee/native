#include <unistd.h>
#include <sys/eventfd.h>
#include <vector>
#include "Channel.h"
#include "Poller.h"
#include "Timer.h"
#include "TimerQueue.h"
#include "EventLoop.h"

__thread EventLoop* t_loopInThisThread = nullptr;

EventLoop::EventLoop () :
    _is_running(false),
    _quit(false),
    _thread_id(std::this_thread::get_id()),
    _wakeup_channel(nullptr)
{
    if (t_loopInThisThread) {
        std::cerr << "EventLoop is created before in thread: " << t_loopInThisThread->_thread_id << std::endl;
    } else {
        t_loopInThisThread = this;
    }

    _poller = new Poller();
    _timer_queue = new TimerQueue(this);

    _wakeup_fd = eventfd(0, EFD_CLOEXEC | EFD_NONBLOCK);
    if (_wakeup_fd != -1) {
        _wakeup_channel = new Channel(_wakeup_fd, this);
        _wakeup_channel->enableReadEvent();
        _wakeup_channel->setReadCallback([this](){
                uint64_t count;
                ::read(_wakeup_fd, &count, sizeof(count));
                //this->handlePendingCallback();    // why?
        });
    } else {
        std::cerr << "create eventfd failed" << std::endl;
    }
}


EventLoop::~EventLoop ()
{
    if (_wakeup_channel) {
        delete _wakeup_channel;
        _wakeup_channel = nullptr;
    }

    if (_timer_queue) {
        delete _timer_queue;
        _timer_queue = nullptr;
    }

    if (_poller) {
        delete _poller;
        _poller = nullptr;
    }
}


void
EventLoop::quit ()
{
    _quit.store(true);
    if (!isInLoopThread()) {    // why?
        wakeup();
    }
}


void
EventLoop::assertInLoopThread ()
{
    if (!isInLoopThread()) {
        assert(0);
    }
}


// can be called in other thread
void
EventLoop::pushCallback (const std::function<void()>& cb)
{
    std::lock_guard<std::mutex> lk(_func_queue_mtx);
    _func_queue.push_back(cb);
}


void
EventLoop::runInLoop (const std::function<void()>& cb)
{
    if (isInLoopThread()) {
        cb();
    } else {
        //std::cout << "not in thread!" << std::endl;
        pushCallback(cb);
        wakeup();
    }
}


void
EventLoop::queueInLoop (const std::function<void()>& cb)
{
    pushCallback(cb);
    wakeup();
}


void
EventLoop::wakeup ()
{
    static uint64_t buf = 1;
    ::write(_wakeup_fd, &buf, sizeof(buf));
}


void
EventLoop::handlePendingCallback ()
{
    // NOTE: minimum critical region
    std::lock_guard<std::mutex> lk(_func_queue_mtx);
    std::vector<std::function<void()>> cbList;
    cbList.swap(_func_queue);

    for (auto&& cb : cbList) {
        cb();
    }
}

void
EventLoop::run ()
{
    assert(!_is_running.load());
    assertInLoopThread();
    _is_running.store(true);

    std::vector<Channel*> triggered_channels;
    while (!_quit.load())
    {
        triggered_channels.clear();
        _poller->poll(2000, triggered_channels);
        for (auto&& ch : triggered_channels) {
            ch->handleEvent();
        }
        handlePendingCallback();
    }

    _is_running.store(false);
}


bool
EventLoop::updateChannel (const Channel* ch)
{
    assert(_poller);
    return _poller->updateChannel(ch);
}


bool
EventLoop::removeChannel (const Channel* ch)
{
    assert(_poller);
    return _poller->removeChannel(ch);
}


void
EventLoop::runAt (const Timestamp& time, const std::function<void()>& cb)
{
    _timer_queue->addTimer(cb, time, MicroSeconds(0));
}


void
EventLoop::runAfter (int us, const std::function<void()>& cb)
{
    runAt(Clock::now() + MicroSeconds(us), cb);
}

void
EventLoop::runEvery (int intervalInMicroSeconds, const std::function<void()>& cb)
{
    auto interval = MicroSeconds(intervalInMicroSeconds);
    auto time = Clock::now() + interval;
    _timer_queue->addTimer(cb, time, interval);
}