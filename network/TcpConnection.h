#pragma once

#include "Channel.h"
#include "InetAddress.h"
#include "Buffer.h"
#include "Socket.h"

#include <string>
#include <functional>

class Slice;
class EventLoop;
class Socket;
class Channel;

class TcpConnection : public std::enable_shared_from_this<TcpConnection>
{
    public:
        using ConnectCallback    = std::function<void(const std::shared_ptr<TcpConnection>&)>;
        using DisconnectCallback = std::function<void(const std::shared_ptr<TcpConnection>&)>;
        using CloseCallback      = std::function<void(const std::shared_ptr<TcpConnection>&)>;
        using MessageCallback    = std::function<void(const std::shared_ptr<TcpConnection>&, Buffer&)>;

        TcpConnection (EventLoop* loop, const std::string& name, int sockfd,
                       const InetAddress local_addr, const InetAddress peer_addr);

        TcpConnection (const TcpConnection&) = delete;
        TcpConnection& operator= (const TcpConnection&) = delete;

        ~TcpConnection ();

        std::string name () const { return _name; }

        void setConnectCallback (const ConnectCallback& cb) { _connect_cb = cb; }
        void setDisconnectCallback (const DisconnectCallback& cb) { _disconnect_cb = cb; }
        void setCloseCallback (const CloseCallback& cb) { _close_cb = cb; }
        void setMessageCallback (const MessageCallback& cb) { _message_cb = cb; }

        void send (const std::string& message);
        void send (const Slice& slice);

        void shutdown ();

        void connectEstablished ();
        void connectDestroy ();

    private:
        void sendInLoop (const Slice& message);
        void shutdownInLoop ();

        void handleReadEvent ();
        void handleWriteEvent ();
        void handleCloseEvent ();
        void handleErrorEvent ();

    private:
        enum class State {
            kConnecting,
            kConnected,
            kDisconnecting,
            kDisconnected
        };

        const char* curStateName () const {
            switch (_state) {
                case State::kConnecting:    return "kConnecting";
                case State::kConnected:     return "kConnected";
                case State::kDisconnecting: return "kDisconnecting";
                case State::kDisconnected:  return "kDisconnected";
                default: return "kErrorState";
            }
        }

        void setState (State s) { _state = s; }

        EventLoop*              _loop;
        std::string             _name;
        Socket                  _socket;
        Channel                 _channel;
        InetAddress             _local_addr;
        InetAddress             _peer_addr;

        State                   _state;
        ConnectCallback         _connect_cb;
        DisconnectCallback      _disconnect_cb;
        CloseCallback           _close_cb;
        MessageCallback         _message_cb;

        Buffer                  _input_buffer;
        Buffer                  _output_buffer;
};
