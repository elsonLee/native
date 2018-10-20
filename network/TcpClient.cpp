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
      _connector(loop, server_addr),
      _next_conn_id(0),
      _connect_cb(nullptr),
      _write_complete_cb(nullptr),
      _message_cb(nullptr)
{
    _connector.setConnectCallback(
            [this] (int sockfd) {
                handleConnectEvent(sockfd);
            });
}

void
TcpClient::connect ()
{
    _connector.start();
}

void
TcpClient::handleConnectEvent (int sockfd)
{
    _loop->assertInLoopThread();
    InetAddress local_addr(sockops::getLocalAddr(sockfd));
    InetAddress peer_addr(sockops::getPeerAddr(sockfd));
    char buf[32];
    snprintf(buf, sizeof(buf), ":%s#%d", peer_addr.toIpPort().c_str(), _next_conn_id);
    ++_next_conn_id;
    std::string conn_name = _name + buf;

    // sockfd will be closed in TcpConnection._socket.~Socket()
    auto connPtr = std::make_shared<TcpConnection>(_loop, conn_name, sockfd,
                                                   local_addr, peer_addr);
    connPtr->setConnectCallback(_connect_cb);
    connPtr->setDisconnectCallback(_disconnect_cb);
    connPtr->setMessageCallback(_message_cb);
    connPtr->setCloseCallback(
            [this](const std::shared_ptr<TcpConnection>& conn){
                     removeConnectionDelayed(conn);
                  });

    connPtr->connectEstablished();
}

void
TcpClient::removeConnectionDelayed (const std::shared_ptr<TcpConnection>& connPtr)
{
    // NOTE: cannot delete TcpConnection here, it will trigger destructor of Channel and current function is
    // called by Channel, so deletion must be delayed
    //::delete conn;
    _loop->queueInLoop([connPtr]{ connPtr->connectDestroy(); /*::delete conn;*/ });
}
