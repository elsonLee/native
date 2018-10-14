#include "Header.h"

#include <stdio.h>
#include <iostream>

void
onConnection (const TcpConnection& conn)
{
    printf("onConnection(): new connection [%s]\n",
            conn.name().c_str());
}


void
onDisconnection (const TcpConnection& conn)
{
    printf("onDisonnection(): del connection [%s]\n",
            conn.name().c_str());
}


void
onMessage (TcpConnection& conn, Buffer& buf)
{
    int len = buf.readableBytes();
    if (len > 0) {
        char tmp[len+1];
        tmp[len] = '\0';
        len = buf.retrieve(&tmp, len);
        conn.send(std::string(tmp));
    } else {
        printf("onMessage(): receive %d bytes from conn [%s]\n",
                len, conn.name().c_str());
    }
}


int main ()
{
    InetAddress listen_addr(5000);
    EventLoop loop;

    TcpServer server("echoServer", &loop, listen_addr);
    server.setConnectionCallback(onConnection);
    server.setDisconnectionCallback(onDisconnection);
    server.setMessageCallback(onMessage);
    server.start();

    loop.run();

    return 0;
}
