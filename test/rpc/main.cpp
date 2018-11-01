#include "Header.h"
#include "service.pb.h"
#include <cassert>
#include <string>
#include <future>
#include <unordered_map>

using namespace google;

class CalculationServiceImpl : public testcase::CalculationService {
    public:
        void SolveSum (protobuf::RpcController* controller,
                       const testcase::RequestSum* request,
                       testcase::ReplySum* response,
                       protobuf::Closure* done) override
        {
            int32_t ret = 0;
            for (int i = 0; i < request->num_size(); i++) {
                ret += request->num(i);
            }
            response->set_result(ret);
            //done->Run();  // FIXME, done is for async
        }

        void SolveMul (protobuf::RpcController* controller,
                       const testcase::RequestMul* request,
                       testcase::ReplyMul* response,
                       protobuf::Closure* done) override
        {
            int32_t ret = 1;
            for (int i = 0; i < request->num_size(); i++) {
                ret *= request->num(i);
            }
            response->set_result(ret);
            //done->Run();  // FIXME, done is for async
        }
};

class RpcChannel : public protobuf::RpcChannel
{
    public:
        RpcChannel (const std::shared_ptr<TcpConnection>& connPtr)
            : _connPtr(connPtr),
              _codec([this](const std::shared_ptr<TcpConnection>& connPtr,
                        std::unique_ptr<protobuf::Message> message)
                    {
                        assert(connPtr == _connPtr);
                        onProtoMessage(connPtr, std::move(message));
                    })
        {}

        void onMessage (const std::shared_ptr<TcpConnection>& connPtr, Buffer& buf)
        {
            std::cout << "[rpclient] onMessage" << std::endl;
            _codec.recvMessage(connPtr, buf);
        }

        void onProtoMessage (const std::shared_ptr<TcpConnection>& connPtr,
                std::unique_ptr<protobuf::Message> messagePtr)
        {
            std::cout << "[rpclient] onProtoMessage" << std::endl;
            // FIXME
            const auto desc = messagePtr->GetDescriptor();
            auto iter = _records.find(desc);
            assert(iter != _records.end());
            auto response = std::get<0>(iter->second);
            auto done = std::get<1>(iter->second);
            response->CopyFrom(*messagePtr.get());
            done->Run();
        }

        void CallMethod (const protobuf::MethodDescriptor* method,
                         protobuf::RpcController* controller,
                         const protobuf::Message* request,
                         protobuf::Message* response,
                         protobuf::Closure* done) override
        {
            if (_records.find(response->GetDescriptor()) != _records.end()) {
                std::cout << "call method later" << std::endl;
            } else {
                _records.emplace(response->GetDescriptor(),
                        std::tuple<protobuf::Message*, protobuf::Closure*>(response, done));
                _codec.sendMessage(_connPtr, *request);
            }
        }
    private:
        std::shared_ptr<TcpConnection>  _connPtr;
        ProtobufCodec                   _codec;
        std::unordered_map<const protobuf::Descriptor*,
            std::tuple<protobuf::Message*, protobuf::Closure*>> _records;
};

class RpcClient : public TcpClient
{
    public:
        RpcClient (EventLoop* loop, const InetAddress& server_addr) :
            TcpClient("rpclient", loop, server_addr)
    {
        setConnectCallback([this](const std::shared_ptr<TcpConnection>& connPtr){ onConnect(connPtr); });
    }

        void onConnect (const std::shared_ptr<TcpConnection>& connPtr)
        {
            std::cout << "[rpclient] onConnect" << std::endl;
            auto chanPtr = std::make_unique<RpcChannel>(connPtr);
            // FIXME:
            connPtr->setMessageCallback([chan = chanPtr.get()](const std::shared_ptr<TcpConnection>& connPtr, Buffer& buf)
                    { chan->onMessage(connPtr, buf); } );
            _promise.set_value(std::move(chanPtr));
        }

        std::future<std::unique_ptr<RpcChannel>> getChannel () { return _promise.get_future(); }

    private:
        std::promise<std::unique_ptr<RpcChannel>> _promise;
};

void handleReply (testcase::ReplySum* reply)
{
    assert(reply);
    std::cout << "Sum: " << reply->result() << std::endl;
}

void handleReply (testcase::ReplyMul* reply)
{
    assert(reply);
    std::cout << "Mul: " << reply->result() << std::endl;
}

int main (int argc, char* argv[])
{
    EventLoopThread loop_thread;
    EventLoop* loop = loop_thread.start();

    // rpcServer
    CalculationServiceImpl service;
    RpcServer server(loop, InetAddress(9981), &service);
    server.start();

    // rpcClient
    RpcClient client(loop, InetAddress(9981));
    client.connect();
    auto chan = client.getChannel();

    auto chanPtr = chan.get();
    assert(chanPtr.get());
    std::cout << "chan: " << chanPtr.get() << std::endl;

    // service
    auto stub = ::new testcase::CalculationService::Stub(chanPtr.get());
    testcase::RequestSum sum_request;
    testcase::RequestMul mul_request;
    testcase::ReplySum sum_reply;
    testcase::ReplyMul mul_reply;

    for (int i = 1; i < 10; i++) {
        sum_request.add_num(i);
        mul_request.add_num(i);
    }

    stub->SolveSum(nullptr, &sum_request, &sum_reply, protobuf::NewCallback(handleReply, &sum_reply));
    stub->SolveMul(nullptr, &mul_request, &mul_reply, protobuf::NewCallback(handleReply, &mul_reply));

    sleep(30);

    return 0;
}
