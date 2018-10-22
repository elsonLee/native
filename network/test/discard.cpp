#include "Header.h"

#include <stdio.h>
#include <iostream>
#include <memory>

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
        //printf("onMessage(): receive %d bytes from conn [%s]: %s\n",
        //        len, conn.name().c_str(), tmp);
#if 0
        conn.send(
                "HTTP/1.1 200 OK\r\n\
                Server: lxServer 0.1\r\n\
                Content-Type: text/html\r\n\
                \n\r\n\
                <html><title>Test</title>\r\n\
                <body>Hello world</body>\r\n\
                </html>\r\n\
                \r\n");
#endif
        //conn.shutdown();
    } else {
        printf("onMessage(): receive %d bytes from conn [%s]\n",
                len, conn->name().c_str());
    }
    //sleep(3);
    //char buf[] = "replay";
    //conn.write(buf, sizeof(buf));
}


int main ()
{
#if 0
    Buffer buffer;
    int cnt;
    cnt = buffer.append("test1", 5);
    cnt = buffer.append(" hello", 6);
    cnt = buffer.append(" world", 6);
    printf("buffer write: %d\n", cnt);
    char tmp[20];
    cnt = buffer.retrieve(tmp, 20); 
    printf("buffer read: %d\n", cnt);
    for (int i = 0; i < cnt; i++) {
        printf("%c", tmp[i]);
    }
    printf("\n");

    return 0;
#endif


    InetAddress listenAddr(9981);
    EventLoop loop;

    TcpServer server("discardServer", &loop, listenAddr);
    server.setConnectCallback(onConnection);
    server.setDisconnectCallback(onDisconnection);
    server.setMessageCallback(onMessage);
    server.start();

    loop.run();

    return 0;
}
