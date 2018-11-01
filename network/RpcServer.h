#pragma once

#include "ProtobufCodec.h"
#include "TcpServer.h"
#include "Utility.h"
#include <unordered_map>

#include <google/protobuf/service.h>

using namespace google;

class RpcServer : public TcpServer
{
    public:

        RpcServer (EventLoop* loop, const InetAddress& listen_addr, protobuf::Service* service);

    private:

        void registerService (protobuf::Service* service);

        void onConnect (const std::shared_ptr<TcpConnection>& connPtr)
        {
            //std::cout << "[rpcserver] onConnect" << std::endl;
            // TODO
            //setMessageCallback([this](const std::shared_ptr<TcpConnection>& connPtr, Buffer& buf){
            //            _codec.recvMessage(connPtr, buf);
            //        });
        }

        void onProtoMessage (const std::shared_ptr<TcpConnection>& connPtr,
                             std::unique_ptr<protobuf::Message> message);

        void doneProtoMessage (const std::shared_ptr<TcpConnection>& connPtr,
                               protobuf::Message* response);

    private:

        protobuf::Service*  _service;

        ProtobufCodec       _codec;

        std::unordered_map<
            const protobuf::Descriptor*, std::string>  _methods;   //! message desc to method_name
};
