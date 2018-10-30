#pragma once

#include "TcpConnection.h"

#include <google/protobuf/descriptor.h>
#include <google/protobuf/message.h>
#include <unordered_map>
#include <memory>
#include <functional>

using namespace google;

template <typename ConcreteMessage>
using DispatcherMessageCallback = std::function<void(const ConcreteMessage& message,
                                                     const std::shared_ptr<TcpConnection>& connPtr)>;

class MessageWrapperBase
{
    public:
        virtual ~MessageWrapperBase () = default;
        virtual void onMessage (const protobuf::Message& message,
                                const std::shared_ptr<TcpConnection>& connPtr);
};

template <typename ConcreteMessage>
class MessageWrapper : public MessageWrapperBase
{
    public:
        MessageWrapper (const DispatcherMessageCallback<ConcreteMessage>& cb)
              : _cb(cb)
        {}

        void onMessage (const protobuf::Message& message,
                        const std::shared_ptr<TcpConnection>& connPtr) override
        {
            const auto& concrete_message = dynamic_cast<ConcreteMessage&>(message);
            _cb(concrete_message, connPtr);
        }

    private:
        DispatcherMessageCallback<ConcreteMessage>   _cb;
};

class ProtobufDispatcher
{
    public:
        template <typename ConcreteMessage>
        void registerMessageCallback (const DispatcherMessageCallback<ConcreteMessage>& cb);

        void onProtobufMessage (const std::shared_ptr<TcpConnection>&,
                                const protobuf::Message& message);

    private:
        std::unordered_map<const protobuf::Descriptor*,
                           std::unique_ptr<MessageWrapperBase>> _map;
};
