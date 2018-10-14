#pragma once

#include "Common.h"
#include "Buffer.h"

#include <map>

class EventLoop;
class InetAddress;
class TcpConnection;
class Acceptor;

class TcpServer
{
    public:
        using ConnectionCallback = std::function<void(const TcpConnection&)>;
        using DisconnectionCallback = std::function<void(const TcpConnection&)>;
        using MessageCallback = std::function<void(TcpConnection&, Buffer&)>;

        explicit TcpServer (const std::string& name, EventLoop* loop, const InetAddress& listen_addr);
        ~TcpServer ();

        void start ();

        void setConnectionCallback (const ConnectionCallback& cb) { _conn_cb = cb; }
        void setDisconnectionCallback (const DisconnectionCallback& cb) { _disconn_cb = cb; }
        void setMessageCallback (const MessageCallback& cb) { _msg_cb = cb; }

    private:
        void newConnection (int sockfd, const InetAddress& peerAddr);
        void removeConnection (TcpConnection* conn);

    private:
        EventLoop*              _loop;
        const std::string       _name;
        Acceptor*               _acceptor;
        ConnectionCallback      _conn_cb{nullptr};
        DisconnectionCallback   _disconn_cb{nullptr};
        MessageCallback         _msg_cb{nullptr};
        bool                    _started;
        int                     _next_conn_id;
        std::map<std::string, std::shared_ptr<TcpConnection>> _connections;
};
