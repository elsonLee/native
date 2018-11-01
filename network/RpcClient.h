#pragma once

#include "RpcChannel.h"
#include "TcpClient.h"

#include <future>
#include <memory>

class RpcClient : public TcpClient
{
    public:

        RpcClient (EventLoop* loop, const InetAddress& server_addr);

        std::future<std::unique_ptr<RpcChannel>> getChannel ()
        { return _promise.get_future(); }

    private:

        void onConnect (const std::shared_ptr<TcpConnection>& connPtr);

    private:

        std::promise<std::unique_ptr<RpcChannel>> _promise;
};
