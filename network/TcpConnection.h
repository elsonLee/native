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
        using ConnectedCallback = std::function<void(const TcpConnection&)>;
        using DisconnectedCallback = std::function<void(const TcpConnection&)>;
        using CloseCallback = std::function<void(TcpConnection*)>;
        using MessageCallback = std::function<void(TcpConnection&, Buffer&)>;

        TcpConnection (EventLoop* loop, const std::string& name, int sockfd,
                       const InetAddress localAddr, const InetAddress peerAddr);

        TcpConnection (const TcpConnection&) = delete;
        TcpConnection& operator= (const TcpConnection&) = delete;

        ~TcpConnection ();

        std::string name () const { return _name; }

        void setOnConnected (const ConnectedCallback& cb) { _connected_cb = cb; }
        void setOnDisconnected (const DisconnectedCallback& cb) { _disconnected_cb = cb; }
        void setOnClose (const CloseCallback& cb) { _close_cb = cb; }
        void setOnMessage (const MessageCallback& cb) { _message_cb = cb; }

        void disconnect ();

        void send (const std::string& message);

        void shutdown ();

        void connectEstablished ();

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
            kConnected,
            kDisconnecting,
            kDisconnected
        };

        void setState (State s) { _state = s; }

        EventLoop*              _loop;
        std::string             _name;
        Socket                  _socket;
        Channel                 _channel;
        InetAddress             _local_addr;
        InetAddress             _peer_addr;

        State                   _state;
        ConnectedCallback       _connected_cb;
        DisconnectedCallback    _disconnected_cb;
        CloseCallback           _close_cb;
        MessageCallback         _message_cb;

        Buffer                  _input_buffer;
        Buffer                  _output_buffer;
};
