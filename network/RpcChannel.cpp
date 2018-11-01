#include "RpcChannel.h"

using namespace google;

RpcChannel::RpcChannel (const std::shared_ptr<TcpConnection>& connPtr)
    : _connPtr(connPtr),
      _codec([this](const std::shared_ptr<TcpConnection>& connPtr,
                    std::unique_ptr<protobuf::Message> message)
                   {
                       assert(connPtr == _connPtr);
                       onProtoMessage(connPtr, std::move(message));
                   })
{}

void
RpcChannel::onProtoMessage (const std::shared_ptr<TcpConnection>& connPtr,
                            std::unique_ptr<protobuf::Message> messagePtr)
{
    const auto desc = messagePtr->GetDescriptor();
    auto iter = _records.find(desc);
    assert(iter != _records.end());
    auto response = std::get<0>(iter->second);
    auto done = std::get<1>(iter->second);
    response->CopyFrom(*messagePtr.get());
    done->Run();
}

void
RpcChannel::CallMethod (const protobuf::MethodDescriptor* method,
                        protobuf::RpcController* controller,
                        const protobuf::Message* request,
                        protobuf::Message* response,
                        protobuf::Closure* done)
{
    if (_records.find(response->GetDescriptor()) != _records.end()) {
        // FIXME
        std::cout << "call method later" << std::endl;
    } else {
        _records.emplace(response->GetDescriptor(),
                std::tuple<protobuf::Message*, protobuf::Closure*>(response, done));
        _codec.sendMessage(_connPtr, *request);
    }
}
