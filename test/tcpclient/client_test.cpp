#include "Header.h"

#include <cstdio>

void connectionCallback (const std::shared_ptr<TcpConnection>& conn)
{
    printf("client connected\n");
}

int main ()
{
    EventLoopThread loop_thread;
    auto loop = loop_thread.start();

    TcpClient client("client", loop, InetAddress(5000));
    client.setConnectCallback(connectionCallback);
    client.connect();

    sleep(10);

    return 0;
}
