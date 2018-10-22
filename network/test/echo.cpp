#include "Header.h"

#include <stdio.h>
#include <iostream>

using TcpConnectionPtr = std::shared_ptr<TcpConnection>;

void
onConnection (const TcpConnectionPtr& conn)
{
    printf("onConnection(): new connection [%s]\n",
            conn->name().c_str());
}


void
onDisconnection (const TcpConnectionPtr& conn)
{
    printf("onDisonnection(): del connection [%s]\n",
            conn->name().c_str());
}


void
onMessage (const TcpConnectionPtr& conn, Buffer& buf)
{
    int len = buf.readableBytes();
    if (len > 0) {
        char tmp[len+1];
        tmp[len] = '\0';
        len = buf.retrieve(&tmp, len);
        conn->send(Slice(tmp));
    } else {
        printf("onMessage(): receive %d bytes from conn [%s]\n",
                len, conn->name().c_str());
    }
}


int main ()
{
    InetAddress listen_addr(5000);
    EventLoop loop;

    TcpServer server("echoServer", &loop, listen_addr);
    server.setConnectCallback(onConnection);
    server.setDisconnectCallback(onDisconnection);
    server.setMessageCallback(onMessage);
    server.start();

    loop.run();

    return 0;
}
