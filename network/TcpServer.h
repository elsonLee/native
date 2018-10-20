#pragma once

#include "Common.h"
#include "Buffer.h"
#include "Acceptor.h"
#include "TcpConnection.h"

#include <map>

class EventLoop;
class InetAddress;
class TcpConnection;
class Acceptor;

class TcpServer
{
    public:
        using ConnectCallback    = std::function<void(const std::shared_ptr<TcpConnection>&)>;
        using DisconnectCallback = std::function<void(const std::shared_ptr<TcpConnection>&)>;
        using MessageCallback    = std::function<void(const std::shared_ptr<TcpConnection>&, Buffer&)>;

        TcpServer (const std::string& name, EventLoop* loop, const InetAddress& listen_addr);
        ~TcpServer () = default;

        void start ();

        void setConnectCallback (const ConnectCallback& cb) { _connect_cb = cb; }
        void setDisconnectCallback (const DisconnectCallback& cb) { _disconnect_cb = cb; }
        void setMessageCallback (const MessageCallback& cb) { _message_cb = cb; }

    private:
        void handleNewConnectionEvent (int peer_fd, const InetAddress& peer_addr);
        void removeConnection (const std::shared_ptr<TcpConnection>& conn);

    private:
        const std::string           _name;
        EventLoop*                  _loop;
        Acceptor                    _acceptor;
        bool                        _started;
        int                         _next_conn_id;

        ConnectCallback             _connect_cb;
        DisconnectCallback          _disconnect_cb;
        MessageCallback             _message_cb;

        // TcpConnection is responsible for data reading/writing, its life time
        // depends on the real connection
        std::map<std::string, std::shared_ptr<TcpConnection>> _connections;
};
