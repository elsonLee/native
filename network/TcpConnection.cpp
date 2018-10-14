#include "Common.h"

#include "EventLoop.h"
#include "Socket.h"
#include "Channel.h"
#include "SocketOps.h"

#include "TcpConnection.h"

TcpConnection::TcpConnection (EventLoop* loop, std::string name, int sockfd,
        const InetAddress local_addr, const InetAddress peer_addr) :
    _event_loop(loop),
    _name(name),
    _socket(Socket(sockfd)),
    _channel(new Channel("sockfd", sockfd, loop)),
    _local_addr(local_addr),
    _peer_addr(peer_addr),
    _state(State::kConnecting)
{
    _channel->setReadCallback(
            [this]() { handleRead(); }
            );
    _channel->setWriteCallback(
            [this]() { handleWrite(); }
            );
    _channel->setCloseCallback(
            [this]() { handleClose(); }
            );
    _channel->enableReadEvent();
    std::cout << "[TcpConnection] create '" << _name << "'" << std::endl;
}


TcpConnection::~TcpConnection ()
{
    if (_channel) {
        _channel->disableAllEvent();
        ::delete _channel;
    }
    std::cout << "[TcpConnection] delete '" << _name << "'" << std::endl;
}


void
TcpConnection::handleRead ()
{
    //char buf[65536] = "";
    //ssize_t n = ::read(_channel->fd(), buf, sizeof(buf));
    int error = 0;
    int n = _input_buffer.readFd(_channel->fd(), error);
    if (n > 0) {
        //std::cout << "[TcpConnection] handleRead " << n << " Bytes" << std::endl;
        _message_cb(*this, _input_buffer);
    } else if (n == 0) {    // received FIN
        std::cout << "[TcpConnection] Client half closed" << std::endl;
        handleClose();
    } else {
        std::cerr << "TcpConnection::handleRead error: " << error << std::endl;
    }
}


void
TcpConnection::handleWrite ()
{
    std::cout << "handleWrite" << std::endl;
    _event_loop->assertInLoopThread();
    if (_channel->isWriteEventOn()) {
        ssize_t n = ::write(_channel->fd(), _output_buffer.peek(),
                _output_buffer.readableBytes());
        if (n > 0) {
            _output_buffer.retrieve(nullptr, n);
            if (_output_buffer.readableBytes() == 0) {
                _channel->disableWriteEvent();
            }
        }
    }
}


void
TcpConnection::handleClose ()
{
    _channel->disableAllEvent();
    assert(_close_cb);
    _close_cb(this);
}


void
TcpConnection::disconnect ()
{
    _channel->disableAllEvent();
    if (_disconnection_cb) {
        _disconnection_cb(*this);
    }
}


void
TcpConnection::send (const std::string& message)
{
    // TODO: check status
    if (_event_loop->isInLoopThread()) {
        sendInLoop(message);
    } else {
        _event_loop->runInLoop([this, &message](){ sendInLoop(message); });
    }
}


void
TcpConnection::sendInLoop (const std::string& message)
{
    _event_loop->assertInLoopThread();
    ssize_t nwrote = 0;
    if (!_channel->isWriteEventOn() && _output_buffer.readableBytes() == 0) {
        nwrote = ::write(_channel->fd(), message.data(), message.size());
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
        if (!_channel->isWriteEventOn()) {
            _channel->enableWriteEvent();
        }
    }

    std::cout << "send " << nwrote << " Bytes" << std::endl;
    //std::cout << "[TcpConnection] send " << message.size() << " Bytes" << std::endl;
}


void
TcpConnection::shutdownInLoop ()
{
    _event_loop->assertInLoopThread();
    if (!_channel->isWriteEventOn()) {
        _socket.shutdownWrite();
    }
}


void
TcpConnection::shutdown ()
{
    _event_loop->runInLoop([this](){ shutdownInLoop(); });
}
