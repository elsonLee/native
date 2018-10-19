#include "Common.h"

#include <iostream>
#include <arpa/inet.h>

#include "InetAddress.h"

#if 0
void
fromIpPort (const char* ip, uint16_t port, struct sockaddr_in* addr)
{
    addr->sin_family = AF_INET;
    addr->sin_port = htons(port);
    if (::inet_pton(AF_INET, ip, &addr->sin_addr) <= 0) {
        std::cout << "fromIpPort faileld" << std::endl;
    }
}
#endif

InetAddress::InetAddress (uint16_t port, bool loopbackOnly)
{
    bzero(&_addr, sizeof(_addr));
    _addr.sin_family = AF_INET;
    in_addr_t ip = loopbackOnly? INADDR_LOOPBACK : INADDR_ANY;
    _addr.sin_addr.s_addr = htonl(ip);
    _addr.sin_port = htons(port);
}

std::string
InetAddress::toIpPort (void) const
{
    char buf[INET_ADDRSTRLEN];
    if (inet_ntop(AF_INET, &_addr.sin_addr, buf, sizeof(buf))) {
        return std::string(buf);
    } else {
        return "toIpPortError";
    }
}
