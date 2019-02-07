#pragma once

#include "EventLoop.h"
#include "InetAddress.h"
#include "Socket.h"
#include "Channel.h"

class Acceptor
{
    public:

        using ConnectionCallback = std::function<void(int sock_fd, const InetAddress&)>;

        Acceptor (EventLoop* loop, const InetAddress& listen_addr);

        Acceptor (const Acceptor&) = delete;

        Acceptor& operator= (const Acceptor&) = delete;

        ~Acceptor ();

        void listen ();

        //! FIXME: sockfd must be closed in callback now
        void setConnectionCallback (const ConnectionCallback& cb) {
            _connection_cb = cb;
        }

    private:

        void handleAcceptEvent();

    private:

        EventLoop*          _loop;

        InetAddress         _listen_addr;

        Socket              _socket;        // FIXME

        Channel             _channel;

        bool                _listening;

        ConnectionCallback  _connection_cb;     // new connection arrived
};
