#include <sys/epoll.h>

#include "EventLoop.h"
#include "Poller.h"
#include "Channel.h"

constexpr int kNoneEvent = 0;
constexpr int kReadEvent = EPOLLIN | EPOLLPRI;
constexpr int kWriteEvent = EPOLLOUT;

Channel::Channel (const std::string& name, int fd, EventLoop* loop) :
    _name(name),
    _fd(fd),
    _loop(loop),
    _events(kNoneEvent), _revents(kNoneEvent),
    _read_cb(nullptr),
    _write_cb(nullptr),
    _close_cb(nullptr),
    _error_cb(nullptr)
{
    //std::cout << "[channel: " << _name << "] create " << _fd << std::endl;
}

Channel::~Channel ()
{
    //! cannot remove here, because sockfd can be used by other component
    //  removeFromLoop();
}

void
Channel::removeFromLoop ()
{
    _loop->assertInLoopThread();
    if (_loop) {
        //std::cout << "[channel: " << _name << "] remove " << _fd `
        //<< " this: " << this << std::endl;
        _loop->removeChannel(this);
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
    return _loop->updateChannel(this);
}

void
Channel::handleEvent ()
{
    // TODO: ????
    if ((_revents & EPOLLHUP) && !(_revents & EPOLLIN)) {
        std::cout << "warning: EPOLLHUP" << std::endl;
        if (_close_cb) {
            _close_cb();
        } else {
            std::cout << "[chan:" << _name << "] closeCallback is not set!" << std::endl;
        }
    }

    // accept conn available -> EPOLLIN
    // normal data available -> EPOLLIN
    // urgent data available -> EPOLLPRI
    // peer close -> EPOLLIN or EPOLLRDHUP
    // NOTE: peer close may not trigger EPOLLRDHUP, so put EPOLLRDHUP
    // here, handle with read, if read return 0, it means peer closed
    if (_revents & (EPOLLIN | EPOLLPRI | EPOLLRDHUP)) {
        if (_read_cb) {
            _read_cb();
        } else {
            std::cout << "[chan:" << _name << "] readCallback is not set!" << std::endl;
        }
    }

    // available for write -> EPOLLOUT
    if (_revents & EPOLLOUT) {
        if (_write_cb) {
            _write_cb();
        } else {
            std::cout << "[chan:" << _name << "] writeCallback is not set!" << std::endl;
        }
    }

    if (_revents & EPOLLERR) {
        if (_error_cb) {
            _error_cb();
        } else {
            std::cout << "[chan:" << _name << "] errorCallback is not set!" << std::endl;
        }
    }
}
