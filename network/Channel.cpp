#include <sys/epoll.h>

#include "EventLoop.h"
#include "Poller.h"
#include "Channel.h"

static constexpr int kNoneEvent = 0;
static constexpr int kReadEvent = EPOLLIN | EPOLLPRI;
static constexpr int kWriteEvent = EPOLLOUT;

Channel::Channel (int fd, EventLoop* event_loop) :
    _fd(fd),
    _event_loop(event_loop),
    _events(kNoneEvent), _revents(kNoneEvent)
{
    std::cout << "[Channel@" << this << "] create " << _fd << std::endl;
}


Channel::~Channel ()
{
    if (_event_loop) {
        std::cout << "[Channel@" << this << "] delete " << _fd << std::endl;
        _event_loop->removeChannel(this);
    }
}


bool
Channel::enableReadEvent ()
{
    //std::cout << "[Channel@" << this << "] enableReadEvent " << _fd << std::endl;
    _events |= kReadEvent;
    return update();
}


bool
Channel::disableReadEvent ()
{
    //std::cout << "[Channel@" << this << "] disableReadEvent " << _fd << std::endl;
    _events &= !kReadEvent;
    return update();
}


bool
Channel::enableWriteEvent ()
{
    //std::cout << "[Channel@" << this << "] enableWriteEvent " << _fd << std::endl;
    _events |= kWriteEvent;
    return update();
}


bool
Channel::disableWriteEvent ()
{
    //std::cout << "[Channel@" << this << "] disableWriteEvent " << _fd << std::endl;
    _events &= !kWriteEvent;
    return update();
}


bool
Channel::isWriteEventOn ()
{
    return (_events & kWriteEvent);
}

bool
Channel::enableAllEvent ()
{
    _events |= (kReadEvent | kWriteEvent);
    return update();
}


bool
Channel::disableAllEvent ()
{
    _events &= !(kReadEvent | kWriteEvent);
    return update();
}


bool
Channel::update ()
{
    return _event_loop->updateChannel(this);
}


void
Channel::handleEvent ()
{
    if ((_revents & EPOLLHUP) && !(_revents & EPOLLIN)) {
        std::cout << "warning: EPOLLHUP" << std::endl;
        if (_close_cb) { _close_cb(); }
    }

    if (_revents & (EPOLLIN | EPOLLPRI | EPOLLRDHUP)) {
        if (_read_cb) { _read_cb(); }
    }

    if (_revents & EPOLLOUT) {
        if (_write_cb) { _write_cb(); }
    }

    if (_revents & EPOLLERR) {
        if (_error_cb) { _error_cb(); }
    }
}
