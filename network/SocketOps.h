#pragma once

#include <endian.h>
#include <arpa/inet.h>
#include <sys/socket.h>

namespace sockops
{
    inline uint16_t
    hostToNetwork16 (uint16_t v) {
        return htobe16(v);
    }

    inline uint32_t
    hostToNetwork32 (uint32_t v) {
        return htobe32(v);
    }

    inline uint64_t
    hostToNetwork64 (uint64_t v) {
        return htobe64(v);
    }

    inline uint16_t
    networkToHost16 (uint16_t v) {
        return be16toh(v);
    }

    inline uint32_t
    networkToHost32 (uint32_t v) {
        return be32toh(v);
    }

    inline uint64_t
    networkToHost64 (uint64_t v) {
        return be64toh(v);
    }

    struct sockaddr_in getLocalAddr (int sockfd);
    struct sockaddr_in getPeerAddr (int sockfd);

    int createNonblockingSocket ();

    int connect (int sockfd, const struct sockaddr* addr);

    void shutdownWrite (int sockfd);

    void close (int sockfd);

    void bind (int sockfd, const struct sockaddr* addr);

    void listen (int sockfd);

    int accept (int sockfd, struct sockaddr_in* addr);

    int getSocketError (int sockfd);
}
