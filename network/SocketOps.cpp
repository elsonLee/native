#include "Common.h"
#include "Utility.h"
#include "SocketOps.h"
#include <unistd.h>

// ipv4
namespace sockops
{

struct sockaddr_in
getLocalAddr (int sockfd)
{
    struct sockaddr_in addr;
    bzero(&addr, sizeof(addr));
    socklen_t len = static_cast<socklen_t>(sizeof(addr));
    if (::getsockname(sockfd, reinterpret_cast<struct sockaddr*>(&addr), &len) < 0) {
        std::cerr << "getLocalAddr failed" << std::endl;
    }
    return addr;
}

int
createNonblockingSocket ()
{
    int fd = ::socket(AF_INET,
                      SOCK_STREAM | SOCK_NONBLOCK | SOCK_CLOEXEC,
                      IPPROTO_TCP);
    if (fd < 0) {
        std::cerr << "socket failed" << std::endl;
    }
    return fd;
}

void
bind (int sockfd, const struct sockaddr* addr)
{
    int ret = ::bind(sockfd, addr, static_cast<socklen_t>(sizeof(struct sockaddr_in6)));
    if (ret < 0) {
        std::cerr << "socket bind failed" << std::endl;
    }
}

void
listen (int sockfd)
{
    int ret = ::listen(sockfd, SOMAXCONN);
    if (ret == -1) {
        switch (errno) {
            case EADDRINUSE:
                std::cout << "socket is already listening on the same port" << std::endl;
                break;
            case EBADF:
            case ENOTSOCK:
            case EOPNOTSUPP:
                std::cout << "::listen: " << strerror(errno) << std::endl;
                break;
            default:
                break;
        }
    }
}

int
accept (int sockfd, struct sockaddr_in* addr)
{
    socklen_t addrlen = static_cast<socklen_t>(sizeof(*addr));
    //! NOTE:
    //  return NON-BLOCK socket_fd directly, or use fcntl
    int connfd = ::accept4(sockfd, static_cast<struct sockaddr*>((void*)addr),
                           &addrlen , SOCK_NONBLOCK | SOCK_CLOEXEC);
    if (connfd < 0)
    {
        std::cerr << "socket accept failed" << std::endl;
        // TODO: error handling
    }
    return connfd;
}

int
connect (int sockfd, const struct sockaddr* addr)
{
    return ::connect(sockfd, addr, static_cast<socklen_t>(sizeof(struct sockaddr_in6)));
}

void
shutdownWrite (int sockfd)
{
    int ret = ::shutdown(sockfd, SHUT_WR);
    if (ret < 0) {
        std::cerr << "socket shutdownWrite failed" << std::endl;
    }
}

void
close (int sockfd)
{
    if (::close(sockfd) < 0) {
        std::cerr << "socket close failed" << std::endl;
    }
}



}
