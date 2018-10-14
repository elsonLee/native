#ifndef ACCEPTOR_H
#define ACCEPTOR_H

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

        ~Acceptor ();

        void listen ();

        void setConnectionCallback (const ConnectionCallback& cb) {
            _connection_cb = cb;
        }

    private:
        void handleRead();

        EventLoop*          _loop;
        Socket              _accept_socket;
        Channel             _accept_channel;
        ConnectionCallback  _connection_cb;
        bool                _listening;
        int                 _idle_fd;
        InetAddress         _listen_addr;
};


#endif
