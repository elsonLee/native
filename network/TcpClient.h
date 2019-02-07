#pragma once

#include "InetAddress.h"
//#include "TcpConnection.h"
#include "Connector.h"
#include "Buffer.h"

#include <mutex>
#include <functional>
#include <string>
#include <memory>

class EventLoop;
class TcpConnection;

class TcpClient
{
    public:

        using ConnectCallback       = std::function<void(const std::shared_ptr<TcpConnection>&)>;
        using DisconnectCallback    = std::function<void(const std::shared_ptr<TcpConnection>&)>;
        using WriteCompleteCallback = std::function<void(const std::shared_ptr<TcpConnection>&)>;
        using MessageCallback       = std::function<void(const std::shared_ptr<TcpConnection>&, Buffer&)>;

        TcpClient (const std::string& name,
                   EventLoop* loop,
                   const InetAddress& server_addr);

        ~TcpClient () = default;

        void connect (void);
        void disconnect (void);

        void setConnectCallback (const ConnectCallback& cb) {
            _connect_cb = cb;
        }

        void setDisconnectCallback (const DisconnectCallback& cb) {
            _disconnect_cb = cb;
        }

        void setWriteCompleteCallback (const WriteCompleteCallback& cb) {
            _write_complete_cb = cb;
        }

        void setMessageCallback (const MessageCallback& cb) {
            _message_cb = cb;
        }

    private:

        void handleConnectEvent (int sockfd);
        void removeConnectionDelayed (const std::shared_ptr<TcpConnection>& shared_conn);

    private:
        std::string                 _name;
        EventLoop*                  _loop;
        Connector                   _connector;
        int                         _next_conn_id;

        ConnectCallback             _connect_cb;
        DisconnectCallback          _disconnect_cb;
        WriteCompleteCallback       _write_complete_cb;
        MessageCallback             _message_cb;

        mutable std::mutex              _connMutex;
        std::shared_ptr<TcpConnection>  _connPtr;
};
