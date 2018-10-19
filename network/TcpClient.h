#pragma once

#include "InetAddress.h"
#include "Connector.h"
#include <functional>
#include <string>
#include <memory>

class EventLoop;
class TcpConnection;

class TcpClient
{
    public:
        using ConnectionCallback = std::function<void(const TcpConnection&)>;
        using CloseCallback = std::function<void(const TcpConnection&)>;
        using WriteCompleteCallback = std::function<void(const TcpConnection&)>;

        TcpClient (const std::string& name,
                   EventLoop* loop,
                   const InetAddress& server_addr);

        ~TcpClient () = default;

        void setConnectionCallback (const ConnectionCallback& cb) {
            _conn_cb = cb;
        }

        void setCloseCallback (const CloseCallback& cb) {
            _close_cb = cb;
        }

        void setWriteCompleteCallback (const WriteCompleteCallback& cb) {
            _write_complete_cb = cb;
        }

        void connect (void);

    private:
        void newConnection (int sockfd);
        void removeConnection (const std::shared_ptr<TcpConnection>& shared_conn);

    private:
        std::string                 _name;
        EventLoop*                  _loop;
        std::unique_ptr<Connector>  _connector;
        int                         _next_conn_id;

        ConnectionCallback          _conn_cb;
        CloseCallback               _close_cb;
        WriteCompleteCallback       _write_complete_cb;

};
