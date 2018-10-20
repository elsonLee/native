#pragma once

#include <memory>
#include <functional>

#include "Channel.h"
#include "InetAddress.h"

class EventLoop;
class Connector
{
    public:
        using ConnectCallback = std::function<void(int sockfd)>;

        Connector (EventLoop* loop, const InetAddress& server_addr);
        ~Connector () = default;

        Connector (const Connector&) = delete;
        Connector& operator= (const Connector&) = delete;

        void setConnectCallback (const ConnectCallback& cb) {
            _connectCallback = cb;
        }

        void start ();
        void startInLoop ();
        //void restart ();
        //void stop ();

    private:
        void connect ();
        void connecting (int sockfd);

        void handleConnectingEvent ();

    private:
        enum class State {
            kConnecting,
            kConnected,
            kDisconnecting,
            kDisconnected
        };

        void setState (State s) { _state = s; }

    private:
        State                       _state;
        EventLoop*                  _loop;
        InetAddress                 _server_addr;
        std::unique_ptr<Channel>    _channel;
        ConnectCallback             _connectCallback;
};
