#pragma once

#include "Slice.h"
#include "Buffer.h"
#include "TcpConnection.h"

#include <google/protobuf/message.h>
#include <memory>

using namespace google;

class ProtobufCodec
{
    public:
        constexpr static size_t kHeaderLen = sizeof(int32_t);

        using ProtoMessageCallback = std::function<void(
                const std::shared_ptr<TcpConnection>&,
                std::unique_ptr<protobuf::Message>)>;   // FIXME: shouldn't use const shared_ptr& here?

        ProtobufCodec (protobuf::Message* prototype,
                       const ProtoMessageCallback& message_cb)
            : _prototype(prototype),
              _message_cb(message_cb)
        {}

        ~ProtobufCodec () = default;

        ProtobufCodec (const ProtobufCodec&) = delete;
        ProtobufCodec& operator= (const ProtobufCodec&) = delete;

        void onRecvMessage (const std::shared_ptr<TcpConnection>&, Buffer&);

        void sendMessage (const std::shared_ptr<TcpConnection>&, const protobuf::Message&);

    private:
        bool parseFromSlice (const Slice& slice, protobuf::Message* message);

        size_t serializeToBuffer (Buffer& buf, const protobuf::Message& message); 

    private:
        const protobuf::Message* _prototype;

        ProtoMessageCallback    _message_cb;
};
