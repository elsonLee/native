#include "Acceptor.h"
#include "SocketOps.h"
#include "Socket.h"
#include "Channel.h"
#include "Common.h"

Acceptor::Acceptor (EventLoop* loop, const InetAddress& listen_addr) :
    _loop(loop),
    _listen_addr(listen_addr),
    _socket(sockops::createNonblockingSocket()),  // _socket.fd will be closed in ~Socket()
    _channel("accept_fd", _socket.fd(), _loop),
    _listening(false)
{
    _channel.setReadCallback([this] { handleAcceptEvent(); });
    _socket.setReuseAddr(true);
    _socket.bindAddress(listen_addr);
}

Acceptor::~Acceptor ()
{
    _channel.disableAllEvent();
    _channel.removeFromLoop();
}

void
Acceptor::listen ()
{
    _loop->assertInLoopThread();
    _listening = true;
    //std::cout << "[acceptor] listen @" << _listen_addr.toPort() << " ..." << std::endl;
    _socket.listen();
    _channel.enableReadEvent();
}

void
Acceptor::handleAcceptEvent ()
{
    //std::cout << "[acceptor] handleAcceptEvent" << std::endl;
    _loop->assertInLoopThread();
    InetAddress peer_addr;
    int peer_fd = _socket.accept(&peer_addr);
    if (peer_fd >= 0)
    {
        if (_connection_cb) {
            _connection_cb(peer_fd, peer_addr);
        } else {
            sockops::close(peer_fd);
        }
    }
    else
    {
        std::cerr << "error in Acceptor::handleRead" << std::endl;
    }
}
