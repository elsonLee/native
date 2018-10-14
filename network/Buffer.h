#pragma once

#include <vector>

class Buffer
{
    public:
        Buffer ();
        ~Buffer ();

        const char* peek () const { return &_buf[_readPos]; }
        int readableBytes () const { return _writePos - _readPos; }

        int retrieve (void* data, int size);
        //std::string retrieveAsString (void* data);
        int append (const void* data, int size);
        int readFd (int fd, int& error);

    private:
        std::vector<char>   _buf;
        int                 _readPos;
        int                 _writePos;
};
