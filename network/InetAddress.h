#pragma once

#include <string>
#include "Common.h"

//void fromIpPort (const char* ip, uint16_t port, struct sockaddr_in* addr);

 class InetAddress
{
    public:
        explicit InetAddress (uint16_t port = 0, bool loopbackOnly = false);

        InetAddress (std::string ip, uint16_t port);

        explicit InetAddress (const struct sockaddr_in& addr) :
            _addr(addr)
        {}

        sa_family_t family () const { return _addr.sin_family; }
        std::string toIp (void) const;
        std::string toIpPort (void) const;
        uint16_t toPort (void) const { return ntohs(_addr.sin_port); }

        const struct sockaddr* getSockAddr () const { return static_cast<struct sockaddr*>((void*)&_addr); }
        void setSockAddr (const struct sockaddr_in& addr) { _addr = addr; }

    private:
        struct sockaddr_in _addr;
};
