#include "Acceptor.h"
#include "SocketOps.h"
#include "Socket.h"
#include "Channel.h"
#include "Common.h"

Acceptor::Acceptor (EventLoop* loop, const InetAddress& listen_addr) :
    _loop(loop),
    _accept_socket(sockops::createNonblockingSocket()),
    _accept_channel(_accept_socket.fd(), loop),
    _listening(false),
    _idle_fd(::open("/dev/null", O_RDONLY | O_CLOEXEC)),
    _listen_addr(listen_addr)
{
    _accept_socket.setReuseAddr(true);
    _accept_socket.bindAddress(listen_addr);
    _accept_channel.setReadCallback(
            std::function<void()>(
                [this]() { handleRead(); }
            ));
}

Acceptor::~Acceptor ()
{
    _accept_channel.disableAllEvent();
    //_acceptChannel.remove();
    ::close(_idle_fd);
}

void
Acceptor::listen ()
{
    _loop->assertInLoopThread();
    _listening = true;
    std::cout << "[Acceptor] listen @" << _listen_addr.toPort() << " ..." << std::endl;
    _accept_socket.listen();
    _accept_channel.enableReadEvent();
}

void
Acceptor::handleRead ()
{
    std::cout << "[Acceptor] handleRead" << std::endl;
    _loop->assertInLoopThread();
    InetAddress addr;
    int connfd = _accept_socket.accept(&addr);
    if (connfd >= 0)
    {
        if (_connection_cb) {
            _connection_cb(connfd, addr);
        } else {
            sockops::close(connfd);
        }
    }
    else
    {
        std::cerr << "in Acceptor::handleRead" << std::endl;
    }
}


