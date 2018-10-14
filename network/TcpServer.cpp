#include "Common.h"

#include "InetAddress.h"
#include "Acceptor.h"
#include "SocketOps.h"
#include "EventLoop.h"
#include "TcpConnection.h"

#include "TcpServer.h"

TcpServer::TcpServer (const std::string& name, EventLoop* loop, const InetAddress& listen_addr) :
    _loop(loop),
    _name(name),
    _acceptor(::new Acceptor(loop, listen_addr)),
    _next_conn_id(0)
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
            [this](int sockfd, const InetAddress& peer_addr)
                  {
                    newConnection(sockfd, peer_addr);
                  }
            );
}


void
TcpServer::newConnection (int sockfd, const InetAddress& peer_addr)
{
    _loop->assertInLoopThread();
    char buf[32];
    snprintf(buf, sizeof(buf), "#%d", _next_conn_id);
    _next_conn_id++;
    std::string conn_name = _name + buf;

    std::cout << "[TcpServer] " << _name
              << " new connection [" << conn_name
              << "]" << std::endl;

    InetAddress local_addr(sockops::getLocalAddr(sockfd));
    auto conn = std::make_shared<TcpConnection>(_loop, conn_name, sockfd, local_addr, peer_addr);
    conn->setConnectionCallback(_conn_cb);
    conn->setDisconnectionCallback(_disconn_cb);
    conn->setCloseCallback([this](TcpConnection* c) { removeConnection(c); });
    conn->setMessageCallback(_msg_cb);

    _connections[conn->name()] = conn;
}

void
TcpServer::removeConnection (TcpConnection* conn)
{
    // NOTE: cannot delete TcpConnection here, it will trigger destructor of Channel and current function is
    // called by Channel, so deletion must be delayed
    //::delete conn;
    auto shared_conn = _connections[conn->name()];
    _loop->queueInLoop([=](){ shared_conn->disconnect(); /*::delete conn;*/ });
    _connections.erase(conn->name());
}
