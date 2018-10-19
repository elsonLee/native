#include "Header.h"

#include <cstdio>

void connectionCallback (const TcpConnection& conn)
{
    printf("client connected\n");
}

int main ()
{
    EventLoopThread loop_thread;
    auto loop = loop_thread.start();

    TcpClient client("client", loop, InetAddress(5000));
    client.setConnectionCallback(connectionCallback);
    client.connect();

    sleep(10);

    return 0;
}
