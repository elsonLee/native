#include "Header.h"

void onMessage (const std::shared_ptr<TcpConnection>& conn, Buffer& buf)
{
    if (buf.readableBytes() > 0) {
        conn->send(Slice(buf.peek(), buf.readableBytes()));
    }
}

int main (int argc, char** argv)
{
    InetAddress listen_addr(9981);

    EventLoop loop;
    TcpServer server("pingpong", &loop, listen_addr);
    server.setMessageCallback(onMessage);

    server.start();

    loop.run();

    return 0;
}
