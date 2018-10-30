#pragma once

#include <vector>

class Buffer
{
    public:
        Buffer ();
        ~Buffer ();

        size_t readableBytes () const { return _writePos - _readPos; }

        const char* peek () const { return &_buf[_readPos]; }

        int16_t peekInt16 () const;
        int16_t readInt16 ();
        int32_t peekInt32 () const;
        int32_t readInt32 ();
        int64_t peekInt64 () const;
        int64_t readInt64 ();

        int retrieve (void* data, int size);
        void clearAll ();
        //std::string retrieveAsString (void* data);
        int append (const void* data, int size);
        int readFd (int fd, int& error);

    private:
        std::vector<char>   _buf;
        int                 _readPos;
        int                 _writePos;
};
