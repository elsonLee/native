#include "Common.h"

#include "ProtobufDispatcher.h"

template <typename ConcreteMessage>
void
ProtobufDispatcher::registerMessageCallback (const DispatcherMessageCallback<ConcreteMessage>& cb)
{
    auto descriptor = ConcreteMessage::descriptor();
    assert(descriptor);

    _map.emplace({descriptor, std::unique_ptr(::new MessageWrapper(cb))});
}

void
ProtobufDispatcher::onProtobufMessage (const std::shared_ptr<TcpConnection>& connPtr,
                                       const protobuf::Message& message)
{
    auto descriptor = message.GetDescriptor();
    assert(descriptor);

    if (_map.find(descriptor) != _map.end()) {
        auto message_wrapper = _map[descriptor].get();
        message_wrapper->onMessage(message, connPtr);
    } else {
        std::cerr << descriptor->full_name() << " is not registered" << std::endl;
    }
}
