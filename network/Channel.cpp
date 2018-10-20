#include <sys/epoll.h>

#include "EventLoop.h"
#include "Poller.h"
#include "Channel.h"

constexpr int kNoneEvent = 0;
constexpr int kReadEvent = EPOLLIN | EPOLLPRI;
constexpr int kWriteEvent = EPOLLOUT;

Channel::Channel (const std::string& name, int fd, EventLoop* event_loop) :
    _name(name),
    _fd(fd),
    _event_loop(event_loop),
    _events(kNoneEvent), _revents(kNoneEvent)
{
    //std::cout << "[Channel: " << _name << "] create " << _fd << std::endl;
}

Channel::~Channel ()
{
    if (_event_loop) {
        //std::cout << "[Channel: " << _name << "] delete " << _fd << std::endl;
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
Channel::isReadEventOn () const
{
    return (_events & kReadEvent);
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
Channel::isWriteEventOn () const
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
    // TODO: ????
    if ((_revents & EPOLLHUP) && !(_revents & EPOLLIN)) {
        std::cout << "warning: EPOLLHUP" << std::endl;
        if (_close_cb) { _close_cb(); }
    }

    // accept conn available -> EPOLLIN
    // normal data available -> EPOLLIN
    // urgent data available -> EPOLLPRI
    // peer close -> EPOLLIN or EPOLLRDHUP
    // NOTE: peer close may not trigger EPOLLRDHUP, so put EPOLLRDHUP
    // here, handle with read, if read return 0, it means peer closed
    if (_revents & (EPOLLIN | EPOLLPRI | EPOLLRDHUP)) {
        if (_read_cb) {
            _is_reading = true;
            _read_cb();
            _is_reading = false;
        }
    }

    // available for write -> EPOLLOUT
    if (_revents & EPOLLOUT) {
        if (_write_cb) {
            _is_writing = true;
            _write_cb();
            _is_writing = false;
        }
    }

    if (_revents & EPOLLERR) {
        if (_error_cb) { _error_cb(); }
    }
}
