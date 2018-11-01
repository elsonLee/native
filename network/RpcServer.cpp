#include "RpcServer.h"

#include <cassert>

RpcServer::RpcServer (EventLoop* loop,
                      const InetAddress& listen_addr,
                      protobuf::Service* service)
    : TcpServer("rpcserver", loop, listen_addr),
      _service(service),
      _codec([this](const std::shared_ptr<TcpConnection>& connPtr,
                    std::unique_ptr<protobuf::Message> message)
            {
                onProtoMessage(connPtr, std::move(message));
            })
{
    setConnectCallback([this](const std::shared_ptr<TcpConnection>& connPtr)
                      { onConnect(connPtr); });
    setMessageCallback([this](const std::shared_ptr<TcpConnection>& connPtr, Buffer& buf)
                      {
                        _codec.recvMessage(connPtr, buf);
                      });

    registerService(service);
}

void
RpcServer::registerService (protobuf::Service* service)
{
    assert(service);
    const auto service_desc = service->GetDescriptor();
    for (int i = 0; i < service_desc->method_count(); i++)
    {
        auto method_desc = service_desc->method(i);
        auto input_desc = method_desc->input_type();
        auto method_name = method_desc->name();
        auto iter = _methods.find(input_desc);
        if (iter == _methods.end()) {
            _methods.emplace(input_desc, method_name);
        } else {
            std::cerr << "method " << iter->second << "and method " <<
                method_name << " has the same output" << std::endl;
        }
    }
}

void
RpcServer::onProtoMessage (const std::shared_ptr<TcpConnection>& connPtr,
                           std::unique_ptr<protobuf::Message> message)
{
    const auto desc = _service->GetDescriptor();
    auto iter = _methods.find(message->GetDescriptor());
    assert(iter != _methods.end());
    const auto method = desc->FindMethodByName(iter->second);
    assert(method);
    auto reply = utility::createMessageByTypeName(method->output_type()->full_name());
    assert(reply);
    _service->CallMethod(method, nullptr, message.get(), reply, nullptr);
    //protobuf::NewCallback(this, &RpcServer::doneProtoMessage, connPtr, &reply));
    doneProtoMessage(connPtr, reply);   // FIXME
}

void
RpcServer::doneProtoMessage (const std::shared_ptr<TcpConnection>& connPtr,
                             protobuf::Message* response)
{
    _codec.sendMessage(connPtr, *response);
    ::delete response;
}

