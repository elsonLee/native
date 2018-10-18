#include <thread>
#include <mutex>
#include <condition_variable>

#include <EventLoop.h>
#include <EventLoopThread.h>

EventLoop*
EventLoopThread::start ()
{
    if (_loop.load()) {
        return _loop.load();
    }

    std::mutex mtx;
    std::condition_variable cv;
    std::thread(
            [this, &cv] {
                EventLoop loop;
                _loop.store(&loop);
                cv.notify_all();    //! status changed
                loop.run();
            }
            ).detach();

    std::unique_lock<std::mutex> lk(mtx);
    cv.wait(lk, [this]{ return _loop.load(); });

    return _loop.load();
}
