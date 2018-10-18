#include "Header.h"

void onMessage (TcpConnection& conn, Buffer& buf)
{
    sleep(6);
    printf("read: %s\n", buf.peek());
    printf("conn name: %s\n", conn.name().c_str());
}

int main ()
{
    EventLoop loop;
    TcpServer server("", &loop, InetAddress(9981));
    server.setMessageCallback(onMessage);
    server.start();

    loop.run();

    return 0;
}
