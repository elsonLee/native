#include "Common.h"

#include "InetAddress.h"
#include "Acceptor.h"
#include "SocketOps.h"
#include "EventLoop.h"
#include "TcpConnection.h"

#include "TcpServer.h"

TcpServer::TcpServer (const std::string& name, EventLoop* loop, const InetAddress& listenAddr) :
    _loop(loop),
    _name(name),
    _acceptor(::new Acceptor(loop, listenAddr)),
    _nextConnId(0)
{

}


TcpServer::~TcpServer ()
{
    if (_acceptor) {
        ::delete _acceptor;
        _acceptor = nullptr;
    }
}


void
TcpServer::start ()
{
    _acceptor->listen();
    _acceptor->setConnectionCallback(
            [this](int sockfd, const InetAddress& peerAddr)
            { newConnection(sockfd, peerAddr); }
            );
}


void
TcpServer::newConnection (int sockfd, const InetAddress& peerAddr)
{
    _loop->assertInLoopThread();
    char buf[32];
    snprintf(buf, sizeof(buf), "#%d", _nextConnId);
    _nextConnId++;
    std::string connName = _name + buf;

    std::cout << "[TcpServer] " << _name
              << " new connection [" << connName
              << "]" << std::endl;

    InetAddress localAddr(sockops::getLocalAddr(sockfd));
    auto conn = std::make_shared<TcpConnection>(_loop, connName, sockfd, localAddr, peerAddr);
    conn->setConnectionCallback(_connCallback);
    conn->setDisconnectionCallback(_disconnCallback);
    conn->setCloseCallback([this](TcpConnection* c) { removeConnection(c); });
    conn->setMessageCallback(_msgCallback);

    _connections[conn->name()] = conn;
}

void
TcpServer::removeConnection (TcpConnection* conn)
{
    // NOTE: cannot delete TcpConnection here, it will trigger destructor of Channel and current function is
    // called by Channel, so deletion must be delayed
    //::delete conn;
    auto sharedConn = _connections[conn->name()];
    _loop->queueInLoop([=](){ sharedConn->disconnect(); /*::delete conn;*/ });
    _connections.erase(conn->name());
}
