#pragma once

#include "InetAddress.h"
#include "Buffer.h"

#include <string>
#include <functional>

class EventLoop;
class Socket;
class Channel;

class TcpConnection
{
    public:
        using ConnectionCallback = std::function<void(const TcpConnection&)>;
        using DisconnectionCallback = std::function<void(const TcpConnection&)>;
        using CloseCallback = std::function<void(TcpConnection*)>;
        using MessageCallback = std::function<void(TcpConnection&, Buffer&)>;

        TcpConnection (EventLoop* loop, std::string name, int sockfd, const InetAddress localAddr, const InetAddress peerAddr);
        TcpConnection (const TcpConnection&) = delete;
        ~TcpConnection ();

        std::string name () const { return _name; }

        void setConnectionCallback (const ConnectionCallback& cb) { _connection_cb = cb; }
        void setDisconnectionCallback (const DisconnectionCallback& cb) { _disconnection_cb = cb; }
        void setCloseCallback (const CloseCallback& cb) { _close_cb = cb; }
        void setMessageCallback (const MessageCallback& cb) { _message_cb = cb; }

        void disconnect ();

        void send (const std::string& message);

        void shutdown ();

    private:

        void sendInLoop (const std::string& message);
        void shutdownInLoop ();

        void handleRead ();
        void handleWrite ();
        void handleClose ();
        void handleError ();

    private:
        enum class State {
            kConnecting,
            kConnected
        };

        void setState (State s) { _state = s; }

        EventLoop*              _event_loop;
        std::string             _name;
        Socket                  _socket;
        Channel*                _channel;
        InetAddress             _local_addr;
        InetAddress             _peer_addr;

        State                   _state;
        ConnectionCallback      _connection_cb;
        DisconnectionCallback   _disconnection_cb;
        CloseCallback           _close_cb;
        MessageCallback         _message_cb;

        Buffer                  _input_buffer;
        Buffer                  _output_buffer;
};
