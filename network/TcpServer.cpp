#include "Common.h"

#include "InetAddress.h"
#include "Acceptor.h"
#include "SocketOps.h"
#include "EventLoop.h"
#include "TcpConnection.h"

#include "TcpServer.h"

TcpServer::TcpServer (const std::string& name, EventLoop* loop, const InetAddress& listen_addr) :
    _name(name),
    _loop(loop),
    _acceptor(_loop, listen_addr),
    _started(false),
    _next_conn_id(0),
    _connect_cb(nullptr),
    _disconnect_cb(nullptr),
    _message_cb(nullptr)
{

}


void
TcpServer::start ()
{
    _acceptor.setConnectionCallback(
            [this](int sockfd, const InetAddress& peer_addr)
                  {
                     handleNewConnectionEvent(sockfd, peer_addr);
                  }
            );
    _acceptor.listen();
}


//! NOTE: peer_fd will be closed after the connection dtor
void
TcpServer::handleNewConnectionEvent (int peer_fd, const InetAddress& peer_addr)
{
    _loop->assertInLoopThread();
    char buf[32];
    snprintf(buf, sizeof(buf), "#%d", _next_conn_id);
    _next_conn_id++;
    std::string conn_name = _name + buf;

    //std::cout << "[TcpServer] " << _name
    //          << " new connection [" << conn_name
    //          << "]" << std::endl;

    InetAddress local_addr(sockops::getLocalAddr(peer_fd));
    auto connPtr = std::make_shared<TcpConnection>(_loop, conn_name, peer_fd, local_addr, peer_addr);

    connPtr->setConnectCallback(_connect_cb);
    connPtr->setDisconnectCallback(_disconnect_cb);
    connPtr->setMessageCallback(_message_cb);
    connPtr->setCloseCallback([this](const std::shared_ptr<TcpConnection>& connPtr) {
                                       removeConnection(connPtr); 
                                    });

    _connections[connPtr->name()] = connPtr;

    _loop->runInLoop([connPtr]{ connPtr->connectEstablished(); });
}


void
TcpServer::removeConnection (const std::shared_ptr<TcpConnection>& connPtr)
{
    // NOTE: cannot delete TcpConnection here, it will trigger destructor of Channel and current function is
    // called by Channel, so deletion must be delayed and run in loop
    //::delete conn;
    _loop->queueInLoop([connPtr]{ connPtr->connectDestroy(); /*::delete conn delayed to loop*/ });
    _connections.erase(connPtr->name());
}
