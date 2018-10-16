#include <stdio.h>
#include "Header.h"

void newConnection1 (int sockfd, const InetAddress& peer_addr)
{
    ::write(sockfd, "How are you?\n", 13);
    sockops::close(sockfd);
}

void newConnection2 (int sockfd, const InetAddress& peer_addr)
{
    ::write(sockfd, "How's it going?\n", 16);
    sockops::close(sockfd);
}

int main ()
{
    EventLoop loop;

    Acceptor acceptor1(&loop, InetAddress(9981));
    acceptor1.setConnectionCallback(newConnection1);
    acceptor1.listen();

    Acceptor acceptor2(&loop, InetAddress(9982));
    acceptor2.setConnectionCallback(newConnection2);
    acceptor2.listen();

    loop.run();

    return 0;
}
