#include "Common.h"

#include "Slice.h"
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
    _channel(_name, sockfd, loop),
    _local_addr(local_addr),
    _peer_addr(peer_addr),
    _state(State::kConnecting)
{
    _channel.setReadCallback([this]{ handleReadEvent(); });
    _channel.setWriteCallback([this]{ handleWriteEvent(); });
    _channel.setCloseCallback([this]{ handleCloseEvent(); });
    //std::cout << "[TcpConnection#" << _name << "] ctor" << std::endl;
}

TcpConnection::~TcpConnection ()
{
    _channel.disableAllEvent();
    _channel.removeFromLoop();
    //std::cout << "[TcpConnection#" << _name << "] dtor" << std::endl;
}

void
TcpConnection::handleReadEvent ()
{
    int error = 0;
    int n = _input_buffer.readFd(_channel.fd(), error);
    if (n > 0) {
        //std::cout << "[tcpconn#" << _name << "] recv " << n << " Bytes" << std::endl;
        assert(_message_cb);
        _message_cb(shared_from_this(), _input_buffer);
    } else if (n == 0) {    // received FIN
        //std::cout << "[TcpConnection] peer half closed" << std::endl;
        handleCloseEvent();
    } else {
        std::cerr << "TcpConnection::handleError: " << strerror(error) << std::endl;
        //handleErrorEvent();
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

    // _disconnect_cb won't be called in connectDestroy if shutdown called first
    // FIXME
    if (_disconnect_cb) {
        _disconnect_cb(shared_from_this());
    } else {
        //std::cout << "[TcpConnection#" << _name << "] no DisconnectCallback" << std::endl;
    }

    if (_close_cb) {
        _close_cb(shared_from_this());
    } else {
        //std::cout << "[TcpConnection#" << _name << "] no CloseCallback" << std::endl;
    }
}

void
TcpConnection::handleErrorEvent ()
{
    int error = sockops::getSocketError(_channel.fd());
    std::cerr << "TcpConnection::handleError: " << strerror(error) << std::endl;
}

void
TcpConnection::send (const std::string& message)
{
    send(Slice(message));
}

void
TcpConnection::send (const Slice& slice)
{
    if (_state == State::kConnected) {
        if (_loop->isInLoopThread()) {
            sendInLoop(slice);
        } else {
            // NOTE: must copy slice!
            _loop->runInLoop([this, str=slice.toString()](){ sendInLoop(str); });
        }
    }
}

//! NOTE: if only partially data is sent successfully, the remaining
//  data will be sent in handleWriteEvent from _outputBuffer,
//  thus the input data can be destroied after this function return
void
TcpConnection::sendInLoop (const Slice& slice)
{
    _loop->assertInLoopThread();
    ssize_t nwrote = 0;
    if (!_channel.isWriteEventOn() && _output_buffer.readableBytes() == 0) {
        nwrote = ::write(_channel.fd(), slice.data(), slice.size());
        if (nwrote >= 0) {
            //std::cout << "[tcpconn#" << _name << "] send " << nwrote <<
            //  " Bytes, remain " << slice.size() - nwrote << " Bytes to send" << std::endl;
            if ((size_t)nwrote < slice.size()) {
                //std::cout << "remain " << slice.size() - nwrote << " bytes to send" << std::endl;
            }
        } else {
            nwrote = 0;
            if (errno == EWOULDBLOCK) {
                // ie. Resource temporarily unavailable
                std::cerr << "[TcpConnection] sendInLoop EWOULDBLOCK: " << strerror(errno) << std::endl;
            } else {
                std::cerr << "[TcpConnection] sendInLoop error: " << strerror(errno) << std::endl;
            }
        }
    }

    if ((size_t)nwrote < slice.size()) {
        _output_buffer.append(slice.data() + nwrote, slice.size() - nwrote);
        if (!_channel.isWriteEventOn()) {
            _channel.enableWriteEvent();
        }
    }
}

void
TcpConnection::connectEstablished ()
{
    _loop->assertInLoopThread();
    assert(_state == State::kConnecting);
    setState(State::kConnected);

    _channel.enableReadEvent();
    if (_connect_cb) {
        _connect_cb(shared_from_this());
    } else {
        //std::cout << "[TcpConnection#" << _name << "] no ConnectCallback" << std::endl;
    }
}

void
TcpConnection::connectDestroy ()
{
    _loop->assertInLoopThread();
    //std::cout << "TcpConnection connectDestroy: " << curStateName() << std::endl;
    if (_state == State::kConnected) {
        setState(State::kDisconnected);

        _channel.disableAllEvent();
        if (_disconnect_cb) {
            _disconnect_cb(shared_from_this());
        } else {
            //std::cout << "[TcpConnection#" << _name << "] no DisconnectCallback" << std::endl;
        }
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
    assert(_state == State::kConnected);
    if (_state == State::kConnected) {
        setState(State::kDisconnecting);
        _loop->runInLoop([this]{ shutdownInLoop(); });
    }
}
