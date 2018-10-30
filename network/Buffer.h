#pragma once

#include <vector>

class Buffer
{
    public:
        Buffer ();
        ~Buffer ();

        size_t readableBytes () const { return _write_pos - _read_pos; }

        const char* peek () const { return &_buf[_read_pos]; }

        int16_t peekInt16 () const;

        int16_t readInt16 ();

        int32_t peekInt32 () const;

        int32_t readInt32 ();

        int64_t peekInt64 () const;

        int64_t readInt64 ();

        int retrieve (void* data, int size);

        int retrieve (int size) { return retrieve(nullptr, size); }

        void clearAll ();

        void ensureWriteableBytes (size_t len);

        void append (const void* data, size_t len);

        int readFd (int fd, int& error);

    private:
        std::vector<char>   _buf;
        size_t              _read_pos;
        size_t              _write_pos;
};
