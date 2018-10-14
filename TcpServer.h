#ifndef TCP_SERVER_H
#define TCP_SERVER_H

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

        explicit TcpServer (const std::string& name, EventLoop* loop, const InetAddress& listenAddr);
        ~TcpServer ();

        void start ();

        void setConnectionCallback (const ConnectionCallback& cb) { _connCallback = cb; }
        void setDisconnectionCallback (const DisconnectionCallback& cb) { _disconnCallback = cb; }
        void setMessageCallback (const MessageCallback& cb) { _msgCallback = cb; }

    private:
        void newConnection (int sockfd, const InetAddress& peerAddr);
        void removeConnection (TcpConnection* conn);

    private:
        EventLoop* _loop;
        const std::string _name;
        Acceptor*   _acceptor;
        ConnectionCallback _connCallback;
        DisconnectionCallback _disconnCallback;
        MessageCallback _msgCallback;
        bool _started;
        int _nextConnId;
        std::map<std::string, std::shared_ptr<TcpConnection>> _connections;
};

#endif
