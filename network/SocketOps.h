#pragma once

#include <arpa/inet.h>
#include <sys/socket.h>

namespace sockops
{
    struct sockaddr_in getLocalAddr (int sockfd);

    int createNonblockingSocket ();

    int connect (int sockfd, const struct sockaddr* addr);

    void shutdownWrite (int sockfd);

    void close (int sockfd);

    void bind (int sockfd, const struct sockaddr* addr);

    void listen (int sockfd);

    int accept (int sockfd, struct sockaddr_in* addr);

    int getSocketError (int sockfd);
}
