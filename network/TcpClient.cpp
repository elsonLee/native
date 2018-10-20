#include "TcpClient.h"

#include "Common.h"
#include "SocketOps.h"
#include "EventLoop.h"
#include "TcpConnection.h"

TcpClient::TcpClient (const std::string& name,
                      EventLoop* loop,
                      const InetAddress& server_addr)
    : _name(name),
      _loop(loop),
      _connector(std::make_unique<Connector>(loop, server_addr)),
      _next_conn_id(0),
      _conn_cb(nullptr),
      _close_cb(nullptr),
      _write_complete_cb(nullptr)
{
    _connector->setNewConnectionCallback(
            [this] (int sockfd) {
                newConnection(sockfd);
            });
}

void
TcpClient::connect ()
{
    _connector->start();
}

void
TcpClient::newConnection (int sockfd)
{
    _loop->assertInLoopThread();
    InetAddress local_addr(sockops::getLocalAddr(sockfd));
    InetAddress peer_addr(sockops::getPeerAddr(sockfd));
    char buf[32];
    snprintf(buf, sizeof(buf), ":%s#%d", peer_addr.toIpPort().c_str(), _next_conn_id);
    ++_next_conn_id;
    std::string conn_name = _name + buf;

    auto shared_conn = std::make_shared<TcpConnection>(_loop,
                                                       conn_name,
                                                       sockfd,
                                                       local_addr,
                                                       peer_addr);
    shared_conn->setConnectCallback(_conn_cb);
    shared_conn->setDisconnectCallback(nullptr);
    shared_conn->setMessageCallback(nullptr);
    shared_conn->setCloseCallback([this, shared_conn](std::shared_ptr<TcpConnection> connPtr){ removeConnection(shared_conn); });

    shared_conn->connectEstablished();
}

void
TcpClient::removeConnection (const std::shared_ptr<TcpConnection>& shared_conn)
{
    // NOTE: cannot delete TcpConnection here, it will trigger destructor of Channel and current function is
    // called by Channel, so deletion must be delayed
    //::delete conn;
    _loop->queueInLoop([shared_conn]{ shared_conn->disconnect(); /*::delete conn;*/ });
}
