#include "Common.h"

#include "EventLoop.h"
#include "Socket.h"
#include "Channel.h"
#include "SocketOps.h"

#include "Connector.h"

Connector::Connector (EventLoop* loop, const InetAddress& server_addr) :
    _state(State::kDisconnected),
    _loop(loop),
    _server_addr(server_addr),
    _channel(nullptr),
    _connectCallback(nullptr)
{

}

void
Connector::start ()
{
    _loop->runInLoop([this]{ startInLoop(); });
}

void
Connector::startInLoop ()
{
    _loop->assertInLoopThread();
    assert(_state == State::kDisconnected);

    connect();
}

void
Connector::stop ()
{
    _loop->runInLoop([this]{ stopInLoop(); });
}

void
Connector::stopInLoop ()
{
    _loop->assertInLoopThread();
    assert(_state == State::kDisconnecting);

    int sockfd = _channel->fd();
    _channel->disableAllEvent();
    _channel->reset(nullptr);
    sockops::close(sockfd);
}

void
Connector::connect ()
{
    _loop->assertInLoopThread();
    int sockfd = sockops::createNonblockingSocket();
    int ret = sockops::connect(sockfd, _server_addr.getSockAddr());
    int saved_errno = (ret == 0)? 0 : errno;
    if (ret < 0) {
        switch (saved_errno)
        {
            case 0:
            case EINPROGRESS:   // socket is nonblocking and connection cannot be completed immediately
            case EINTR:         // system call was interrupted by signal
            case EISCONN:       // socket is already connected
                connecting(sockfd);
                break;

            case EAGAIN:        // insufficient entries in the routing cache
            case EADDRINUSE:    // local address is already in use
            case EADDRNOTAVAIL: // sockfd had not previously been bound to an address
            case ECONNREFUSED:  // no one listening on the remote address
            case ENETUNREACH:   // network is unreachable
            case ETIMEDOUT:     // timeout while attemping connection
                break;

            case EACCES:
            case EAFNOSUPPORT:
            case EALREADY:      // previous connection has not yet been completed
            case EBADF:         
            case EFAULT:        // socket address is outside user's address space
            case ENOTSOCK:      // sockfd does not refer to a socket
            case EPROTOTYPE:    // socket type does not support the requested protocol
            default:
                std::cerr << "Connector::connect: " << strerror(saved_errno) << std::endl;
        }
    }
}

void
Connector::connecting (int sockfd)
{
    setState(State::kConnecting);
    assert(!_channel);

    _channel.reset(new Channel("connector", sockfd, _loop));
    _channel->setWriteCallback([this]{ handleConnectingEvent(); });

    _channel->enableWriteEvent();
}

void
Connector::handleConnectingEvent ()
{
    assert(_state == State::kConnecting);
    _loop->assertInLoopThread();

    _channel->disableAllEvent();
    int sockfd = _channel->fd();
    _channel.reset(nullptr);

    int err = sockops::getSocketError(sockfd);
    if (err) {
        sockops::close(sockfd);
    } else {
        setState(State::kConnected);
        std::cout << "Connector: connected" << std::endl;
        if (_connectCallback) {
            _connectCallback(sockfd);   // close sockfd will be handled in callback
        }
    }
}
