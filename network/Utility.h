#pragma once

#include <iostream>
#include <cassert>

class FileDescriptor
{
    public:
        explicit FileDescriptor (int fd) :
            _fd(fd)
        {
            assert(_fd > 0);
        }

        ~FileDescriptor () {
            ::close(_fd);
        }

        int fd () const { return _fd; }

    private:
        int     _fd;
};
