#include "RpcClient.h"

RpcClient::RpcClient (EventLoop* loop,
                      const InetAddress& server_addr)
    : TcpClient("rpclient", loop, server_addr)
{
    setConnectCallback([this](const std::shared_ptr<TcpConnection>& connPtr)
                             { onConnect(connPtr); });
}

void
RpcClient::onConnect (const std::shared_ptr<TcpConnection>& connPtr)
{
    auto chanPtr = std::make_unique<RpcChannel>(connPtr);
    // FIXME:
    connPtr->setMessageCallback(
            [chan = chanPtr.get()](const std::shared_ptr<TcpConnection>& connPtr, Buffer& buf)
                                  { chan->onMessage(connPtr, buf); } );
    _promise.set_value(std::move(chanPtr));
}
