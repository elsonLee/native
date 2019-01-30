#include <sys/epoll.h>
#include <unistd.h>
#include <string.h>
#include <stdio.h>

#include <cassert>
#include <iostream>

#include "Channel.h"
#include "Poller.h"

Poller::Poller ()
{
    // NOTE: When all file descriptors referring to an epoll instance have been closed,
    // the kernel destroys the instance and releases the associated resources for reuse.
    _fd = epoll_create1(0);     // won't do exec
    if (_fd < 0) {
        std::cerr << "epoll create failed" << std::endl;
    }
}


Poller::~Poller ()
{
    if (_fd >= 0) {
        close(_fd);
    }
}


bool
Poller::createOrChangeEvent (int op, int event_fd, uint32_t events)
{
    assert(event_fd >= 0);

    struct epoll_event ev;
    bzero(&ev, sizeof(ev));
    ev.events = events;
    ev.data.fd = event_fd;

    return epoll_ctl(_fd, op, event_fd, &ev) == 0;
}


bool
Poller::removeEvent (int event_fd) {
    assert(event_fd >= 0);

    struct epoll_event ev;

    return epoll_ctl(_fd, EPOLL_CTL_DEL, event_fd, &ev) == 0;
}


bool
Poller::registerChannel (const Channel* ch)
{
    assert(ch);
    assert(_fd >= 0);

    bool ret = createOrChangeEvent(EPOLL_CTL_ADD, ch->fd(), ch->events());
    if (!ret) {
        std::cerr << "[Poller] registerChannel failed: " << ch->name() <<
            " : " << strerror(errno) << std::endl;
    } else {
        //std::cout << "[Poller] registerChannel succ: " << ch->name() << std::endl;
    }

    return ret;
}


bool
Poller::modifyChannel (const Channel* ch)
{
    assert(ch);
    assert(_fd >= 0);

    bool ret = createOrChangeEvent(EPOLL_CTL_MOD, ch->fd(), ch->events());
    if (!ret) {
        std::cerr << "[Poller] modifyChannel failed" << ch->name() <<
            " : " << strerror(errno) << std::endl;
    } else {
        //std::cout << "[Poller] registerChannel succ: " << ch->name() << std::endl;
    }

    return ret;
}


bool
Poller::unregisterChannel (const Channel* ch)
{
    assert(ch);
    assert(_fd >= 0);

    struct epoll_event ev;

    bool ret = removeEvent(ch->fd());
    if (!ret) {
        std::cerr << "[Poller] unregisterChannel failed: " << ch->name() <<
            " : " << strerror(errno) << std::endl;
    } else {
        //std::cout << "[Poller] unregisterChannel succ: " << ch->name() << std::endl;
    }

    return ret;
}


bool
Poller::updateChannel (const Channel* ch)
{
    assert(ch);
    assert(_fd >= 0);

    bool ret = false;

    if (_channel_map.find(ch->fd()) != _channel_map.end()) {
        //if (ch->events() == 0) {
        //    ret = removeChannel(ch);
        //} else {
            ret = modifyChannel(ch);
        //}
    } else {
        ret = registerChannel(ch);
        if (ret) {
            _channel_map.emplace(ch->fd(), const_cast<Channel*>(ch));
        }
    }

    return ret;
}


bool
Poller::removeChannel (const Channel* ch)
{
    bool ret = unregisterChannel(ch);
    if (ret) {
        _channel_map.erase(ch->fd());
    }
    return ret;
}


void
Poller::poll (int timeout_in_mills, std::vector<Channel*>& triggered_channels)
{
    int max_num_events = _channel_map.size();
    std::vector<struct epoll_event> revents(max_num_events);
    int num_events = epoll_wait(_fd, &*revents.begin(), _channel_map.size(), timeout_in_mills);
    if (num_events > 0) {
        for (int i = 0; i < num_events; i++)
        {
            auto ev = revents[i];
            int fd = ev.data.fd;

            auto iter = _channel_map.find(fd);
            assert(iter != _channel_map.end());
            auto ch = iter->second;
            assert(ch);
            ch->setRevents(ev.events);
            triggered_channels.push_back(ch);
        }
    } else if (num_events == 0) {
        //std::cout << "nothing happened" << std::endl;
    }
    else {
        if (errno == EINTR) {
            // timeout
        } 
        else {
            //std::cerr << "poll error: " << strerror(errno) << std::endl;
            fprintf(stderr, "poll error[ret:%d, max:%lu]: %s\n",
                    num_events, _channel_map.size(), strerror(errno));
            exit(-1);
        }
    }
}
