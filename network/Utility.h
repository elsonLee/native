#pragma once

#include <unistd.h>
#include <iostream>
#include <cassert>

namespace google
{
    namespace protobuf
    {
        class Message;
    }
}

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

namespace utility
{


google::protobuf::Message*
createMessageByTypeName (const std::string& type_name);


}
