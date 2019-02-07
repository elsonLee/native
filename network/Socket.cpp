#include "SocketOps.h"
#include "InetAddress.h"
#include "Common.h"

#include "Socket.h"

#include <netinet/tcp.h>

Socket::~Socket()
{
    sockops::close(_sockfd);
}

void
Socket::setReuseAddr (bool on)
{
    int optval = on ? 1 : 0;
    ::setsockopt(_sockfd, SOL_SOCKET, SO_REUSEADDR,
                 &optval, static_cast<socklen_t>(sizeof(optval)));
}

void
Socket::setTcpNoDelay (bool on)
{
    int optval = on ? 1 : 0;
    ::setsockopt(_sockfd, IPPROTO_TCP, TCP_NODELAY,
                 &optval, static_cast<socklen_t>(sizeof(optval)));
}

void
Socket::bindAddress (const InetAddress& addr)
{
    sockops::bind(_sockfd, addr.getSockAddr());
}

void
Socket::listen (void)
{
    sockops::listen(_sockfd);
}

int
Socket::accept (InetAddress* peerAddr)
{
    struct sockaddr_in addr;
    bzero(&addr, sizeof(addr));
    int connfd = sockops::accept(_sockfd, &addr);
    if (connfd >= 0)
    {
        peerAddr->setSockAddr(addr);
    }
    return connfd;
}

void
Socket::shutdownWrite ()
{
    sockops::shutdownWrite(_sockfd);
}

