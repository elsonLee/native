#pragma once

#include "ProtobufCodec.h"

#include <memory>
#include <unordered_map>

#include <google/protobuf/service.h>

class Buffer;
class TcpConnection;

class RpcChannel : public google::protobuf::RpcChannel
{
    public:

        RpcChannel (const std::shared_ptr<TcpConnection>& connPtr);

        void onMessage (const std::shared_ptr<TcpConnection>& connPtr, Buffer& buf)
        {
            _codec.recvMessage(connPtr, buf);
        }

    private:

        void onProtoMessage (const std::shared_ptr<TcpConnection>& connPtr,
                             std::unique_ptr<google::protobuf::Message> messagePtr);

        void CallMethod (const google::protobuf::MethodDescriptor* method,
                         google::protobuf::RpcController* controller,
                         const google::protobuf::Message* request,
                         google::protobuf::Message* response,
                         google::protobuf::Closure* done) override;

    private:

        std::shared_ptr<TcpConnection>  _connPtr;

        ProtobufCodec                   _codec;

        std::unordered_map<const google::protobuf::Descriptor*,
            std::tuple<google::protobuf::Message*, google::protobuf::Closure*>> _records;
};
