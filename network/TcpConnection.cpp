#include "Common.h"

#include "EventLoop.h"
#include "Socket.h"
#include "Channel.h"
#include "SocketOps.h"

#include "TcpConnection.h"

TcpConnection::TcpConnection (EventLoop* loop, const std::string& name, int sockfd,
                              const InetAddress local_addr, const InetAddress peer_addr) :
    _loop(loop),
    _name(name),
    _socket(Socket(sockfd)),    // sockfd will be closed in dtor
    _channel("connection", sockfd, loop),
    _local_addr(local_addr),
    _peer_addr(peer_addr),
    _state(State::kConnecting)
{
    _channel.setReadCallback([this]{ handleReadEvent(); });
    _channel.setWriteCallback([this]{ handleWriteEvent(); });
    _channel.setCloseCallback([this]{ handleCloseEvent(); });
    std::cout << "[TcpConnection] create '" << _name << "'" << std::endl;
}

TcpConnection::~TcpConnection ()
{
    _channel.disableAllEvent();
    std::cout << "[TcpConnection] delete '" << _name << "'" << std::endl;
}

void
TcpConnection::handleReadEvent ()
{
    int error = 0;
    int n = _input_buffer.readFd(_channel.fd(), error);
    if (n > 0) {
        std::cout << "[TcpConnection] handleRead " << n << " Bytes" << std::endl;
        _message_cb(shared_from_this(), _input_buffer);
    } else if (n == 0) {    // received FIN
        std::cout << "[TcpConnection] Client half closed" << std::endl;
        handleCloseEvent();
    } else {
        // TODO: error handling
        std::cerr << "TcpConnection::handleRead error: " << error << std::endl;
    }
}

void
TcpConnection::handleWriteEvent ()
{
    _loop->assertInLoopThread();
    if (_channel.isWriteEventOn()) {
        ssize_t n = ::write(_channel.fd(), _output_buffer.peek(),
                            _output_buffer.readableBytes());
        if (n > 0) {
            _output_buffer.retrieve(nullptr, n);
            if (_output_buffer.readableBytes() == 0) {
                _channel.disableWriteEvent();
            }
        }
    }
}

void
TcpConnection::handleCloseEvent ()
{
    _loop->assertInLoopThread();
    _channel.disableAllEvent();
    assert(_close_cb);
    _close_cb(shared_from_this());
}

void
TcpConnection::send (const std::string& message)
{
    if (_state == State::kConnected) {
        if (_loop->isInLoopThread()) {
            sendInLoop(message);
        } else {
            _loop->runInLoop([this, &message](){ sendInLoop(message); });
        }
    }
}

void
TcpConnection::sendInLoop (const std::string& message)
{
    _loop->assertInLoopThread();
    ssize_t nwrote = 0;
    if (!_channel.isWriteEventOn() && _output_buffer.readableBytes() == 0) {
        nwrote = ::write(_channel.fd(), message.data(), message.size());
        if (nwrote >= 0) {
            if ((size_t)nwrote < message.size()) {
                std::cout << "need write more data" << std::endl;
            }
        } else {
            nwrote = 0;
            std::cerr << "[TcpConnection] sendInLoop error" << std::endl;
        }
    }

    if ((size_t)nwrote < message.size()) {
        nwrote = _output_buffer.append(message.data() + nwrote, message.size() - nwrote);
        if (!_channel.isWriteEventOn()) {
            _channel.enableWriteEvent();
        }
    }

    std::cout << "send " << nwrote << " Bytes" << std::endl;
    //std::cout << "[TcpConnection] send " << message.size() << " Bytes" << std::endl;
}

void
TcpConnection::connectEstablished ()
{
    _loop->assertInLoopThread();
    assert(_state == State::kConnecting);
    setState(State::kConnected);

    _channel.enableReadEvent();
    _connect_cb(shared_from_this());
}

void
TcpConnection::connectDestroy ()
{
    _loop->assertInLoopThread();
    if (_state == State::kConnected) {
        setState(State::kDisconnected);

        _channel.disableAllEvent();
        _disconnect_cb(shared_from_this());
    }
}

void
TcpConnection::shutdownInLoop ()
{
    _loop->assertInLoopThread();
    if (!_channel.isWriteEventOn()) {
        _socket.shutdownWrite();
    }
}

void
TcpConnection::shutdown ()
{
    if (_state == State::kConnected) {
        setState(State::kDisconnecting);
        _loop->runInLoop([this]{ shutdownInLoop(); });
    }
}
