#include "Header.h"

#include <memory>
#include <vector>
#include <iostream>

using TcpConnectionPtr = std::shared_ptr<TcpConnection>;

class Client;
class SessionManager;

class Session
{
    public:
        Session (EventLoop* loop,
                 const InetAddress& server_addr,
                 SessionManager* mgr)
            : _client("session", loop, server_addr),
              _mgr(mgr),
              _bytes_read(0),
              _bytes_write(0),
              _message_read(0)
        {
            _client.setConnectCallback(
                    [this](const TcpConnectionPtr& conn) {
                            onConnection(conn);
                          });
            _client.setDisconnectCallback(
                    [this](const TcpConnectionPtr& connPtr) {
                            //_mgr->onDisconnection(connPtr);
                            onDisconnection(connPtr);
                          });
            _client.setMessageCallback(
                    [this](const TcpConnectionPtr& connPtr, Buffer& buf) {
                            onMessage(connPtr, buf);
                          });
        }

        ~Session () = default;

        void start () { _client.connect(); }

        void stop () { _client.disconnect(); }

        int64_t bytes_read () const { return _bytes_read; }

        int64_t bytes_write () const { return _bytes_write; }

    private:
        void onConnection (const TcpConnectionPtr& connPtr);
        void onDisconnection (const TcpConnectionPtr& connPtr);

        void onMessage (const TcpConnectionPtr& connPtr, Buffer& buf);

    private:
        TcpClient       _client;
        SessionManager* _mgr;

        int64_t         _bytes_read;
        int64_t         _bytes_write;
        int64_t         _message_read;
};

void
Session::onMessage (const TcpConnectionPtr& conn, Buffer& buf)
{
    _message_read += 1;
    _bytes_read += buf.readableBytes();
    _bytes_write += buf.readableBytes();
    conn->send(Slice(buf.peek(), buf.readableBytes()));
}

class SessionManager
{
    public:
        SessionManager (EventLoop* loop,
                        const InetAddress& server_addr,
                        int block_size,
                        int timeout)
            : _loop(loop),
             _timeout(timeout)
        {
            _loop->runAfter(timeout, [this]{ handleTimeout(); });
            for (int i = 0; i < block_size; i++) {
                _message.push_back(static_cast<char>(i % 128));
            }

            auto session = std::make_unique<Session>(loop, server_addr, this);
            session->start();
            _sessions.emplace_back(std::move(session));
        }

        void onDisconnection (const std::shared_ptr<TcpConnection>& connPtr)
        {
            std::cout << "onDisconnection here" << std::endl;
            for (auto& session : _sessions) {
                printf("read_bytes: %ld, write_bytes: %ld\n",
                        session->bytes_read(),
                        session->bytes_write());
                printf("%.2lf MiB/s throughput\n",
                        static_cast<double>(session->bytes_read() * 10e6)/(_timeout * 1024 * 1024));
            }
        }

    const std::string& message () const {
        return _message;
    }

    private:

        void quit () {
            _loop->queueInLoop([this]{ _loop->quit(); });
        }

        void handleTimeout ()
        {
            std::for_each(_sessions.begin(), _sessions.end(),
                    [](const std::unique_ptr<Session>& session) {
                        session->stop(); 
                    });
        }

    private:
        EventLoop*  _loop;
        int         _timeout;
        std::string _message;
        std::vector<std::unique_ptr<Session>> _sessions;
};

void
Session::onConnection (const TcpConnectionPtr& connPtr)
{
    connPtr->send(_mgr->message());
}

void
Session::onDisconnection (const TcpConnectionPtr& connPtr)
{
    _mgr->onDisconnection(connPtr);
}

int main (int argc, char** argv)
{
    EventLoop loop;
    InetAddress server_addr(9981);

    SessionManager session_mgr(&loop, server_addr, 128, 1000);

    loop.run();

    return 0;
}
