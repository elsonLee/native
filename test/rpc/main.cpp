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
