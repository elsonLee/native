#pragma once

class InetAddress;

class Socket
{
    public:
        explicit Socket (int sockfd) :
            _sockfd(sockfd)
        {}

        ~Socket();

        int fd() const { return _sockfd; }

        void bindAddress (const InetAddress& addr);
        void listen (void);

        int accept (InetAddress* addr);

        void shutdownWrite ();

        void setReuseAddr (bool on);
        //void setReusePort (bool on);

    private:
        const int _sockfd;

};
