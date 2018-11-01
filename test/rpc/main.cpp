#include "Header.h"
#include "sum_service.pb.h"
#include <cassert>
#include <string>
#include <future>
#include <unordered_map>

using namespace google;

class SumServiceImpl : public testcase::SumService {
    public:
        void Solve (protobuf::RpcController* controller,
                    const testcase::Request* request,
                    testcase::Reply* response,
                    protobuf::Closure* done) override
        {
            int32_t sum = 0;
            for (int i = 0; i < request->num_size(); i++) {
                sum += request->num(i);
            }
            response->set_sum(sum);
            //done->Run();  // FIXME, done is for async
        }
};

class RpcServer : public TcpServer
{
    public:
        RpcServer (EventLoop* loop, const InetAddress& listen_addr, protobuf::Service* service)
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
            }

        void onConnect (const std::shared_ptr<TcpConnection>& connPtr)
        {
            std::cout << "[rpcserver] onConnect" << std::endl;
            // TODO
            //setMessageCallback([this](const std::shared_ptr<TcpConnection>& connPtr, Buffer& buf){
            //            _codec.recvMessage(connPtr, buf);
            //        });
        }

        void onProtoMessage (const std::shared_ptr<TcpConnection>& connPtr,
                std::unique_ptr<protobuf::Message> message)
        {

            std::cout << "[rpcserver] onProtoMessage" << std::endl;
            const auto desc = _service->GetDescriptor();
            const auto method = desc->FindMethodByName("Solve");
            assert(method);
            testcase::Reply reply;
            _service->CallMethod(method, nullptr, message.get(), &reply, nullptr);
            //protobuf::NewCallback(this, &RpcServer::doneProtoMessage, connPtr, &reply));
            doneProtoMessage(connPtr, reply);   // FIXME
        }

        void doneProtoMessage (const std::shared_ptr<TcpConnection>& connPtr,
                protobuf::Message& response)
        {
            std::cout << "[rpcserver] doneProtoMessage" << std::endl;
            _codec.sendMessage(connPtr, response);
        }

    private:
        protobuf::Service*  _service;
        ProtobufCodec       _codec;
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
            auto response = dynamic_cast<testcase::Reply*>(std::get<0>(_record));
            auto done = std::get<1>(_record);
            auto message = dynamic_cast<testcase::Reply*>(messagePtr.get());
            response->set_sum(message->sum());
            done->Run();
        }

        void CallMethod (const protobuf::MethodDescriptor* method,
                protobuf::RpcController* controller,
                const protobuf::Message* request,
                protobuf::Message* response,
                protobuf::Closure* done) override
        {
            _record = {response, done};
            _codec.sendMessage(_connPtr, *request);
        }
    private:
        std::shared_ptr<TcpConnection>  _connPtr;
        ProtobufCodec                   _codec;
        std::tuple<protobuf::Message*, protobuf::Closure*> _record;
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

void handleReply (testcase::Reply* reply)
{
    assert(reply);
    std::cout << "Sum: " << reply->sum() << std::endl;
}

int main (int argc, char* argv[])
{
    EventLoopThread loop_thread;
    EventLoop* loop = loop_thread.start();

    // rpcServer
    SumServiceImpl sum_service;
    RpcServer server(loop, InetAddress(9981), &sum_service);
    server.start();

    // rpcClient
    RpcClient client(loop, InetAddress(9981));
    client.connect();
    auto chan = client.getChannel();

    auto chanPtr = chan.get();
    assert(chanPtr.get());
    std::cout << "chan: " << chanPtr.get() << std::endl;

    // service
    auto service = ::new testcase::SumService::Stub(chanPtr.get());
    testcase::Request request;
    for (int i = 0; i < 10; i++) {
        request.add_num(i);
    }
    testcase::Reply reply;
    service->Solve(nullptr, &request, &reply, protobuf::NewCallback(handleReply, &reply));

    sleep(300);

    return 0;
}
