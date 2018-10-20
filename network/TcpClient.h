#pragma once

#include "InetAddress.h"
#include "Connector.h"
#include "Buffer.h"

#include <functional>
#include <string>
#include <memory>

class EventLoop;
class TcpConnection;

class TcpClient
{
    public:
        using ConnectCallback       = std::function<void(const std::shared_ptr<TcpConnection>&)>;
        using CloseCallback         = std::function<void(const std::shared_ptr<TcpConnection>&)>;
        using WriteCompleteCallback = std::function<void(const std::shared_ptr<TcpConnection>&)>;
        using MessageCallback       = std::function<void(const std::shared_ptr<TcpConnection>&, Buffer&)>;

        TcpClient (const std::string& name,
                   EventLoop* loop,
                   const InetAddress& server_addr);

        ~TcpClient () = default;

        void setConnectCallback (const ConnectCallback& cb) {
            _connect_cb = cb;
        }

        void setCloseCallback (const CloseCallback& cb) {
            _close_cb = cb;
        }

        void setWriteCompleteCallback (const WriteCompleteCallback& cb) {
            _write_complete_cb = cb;
        }

        void setMessageCallback (const MessageCallback& cb) {
            _message_cb = cb;
        }

    private:
        void connect (void);
        void disconnect (void);

        void handleConnectEvent (int sockfd);
        void removeConnectionDelayed (const std::shared_ptr<TcpConnection>& shared_conn);

    private:
        std::string                 _name;
        EventLoop*                  _loop;
        Connector                   _connector;
        int                         _next_conn_id;

        ConnectCallback             _connect_cb;
        CloseCallback               _close_cb;
        WriteCompleteCallback       _write_complete_cb;
        MessageCallback             _message_cb;
};
