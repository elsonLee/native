#include <sys/timerfd.h>
#include <stdio.h>

#include <cassert>
#include "Header.h"

void timeout (EventLoop* loop)
{
    printf("Timeout!\n");
    loop->quit();
}

class Timeout
{
    public:
        Timeout (EventLoop* loop) :
            _fd(::timerfd_create(CLOCK_MONOTONIC, TFD_NONBLOCK | TFD_CLOEXEC)),
            _channel("timerfd", _fd.fd(), loop)
        {
            _channel.setReadCallback([loop] { timeout(loop); });
            _channel.enableReadEvent();
        }

        ~Timeout ()  = default;

        void setTimeout (int secs) {
            struct itimerspec howlong;
            bzero(&howlong, sizeof(howlong));
            howlong.it_value.tv_sec = secs;
            ::timerfd_settime(_fd.fd(), 0, &howlong, NULL);
        }

    private:
        FileDescriptor _fd;
        Channel        _channel;
};

int main ()
{
    EventLoop loop;
    Timeout timer(&loop);
    timer.setTimeout(5);

    loop.run();

    return 0;
}
