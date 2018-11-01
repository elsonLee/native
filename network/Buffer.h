#pragma once

#include <vector>
#include <cassert>

class Buffer
{
    public:
        Buffer ();

        ~Buffer ();

        bool empty () const { return readableBytes() == 0; }

        size_t readableBytes () const { return _write_pos - _read_pos; }

        const char* peek () const { return _buf.data() + _read_pos; }

        // FIXME: shouldn't expose to public API
        const char* writePos () const { return _buf.data() + _write_pos; }
        char* writePos () { return _buf.data() + _write_pos; }

        // FIXME: shouldn't expose to public API
        void writeSize (size_t len) {
            assert(_write_pos + len <= _buf.size());
            _write_pos += len;
        }

        void readSize (size_t len) {
            assert(_read_pos + len <= _write_pos);
            _read_pos += len;
        }

        int16_t peekInt16 () const;

        int16_t readInt16 ();

        int32_t peekInt32 () const;

        int32_t readInt32 ();

        int64_t peekInt64 () const;

        int64_t readInt64 ();

        std::string readString (size_t len);

        int retrieve (void* data, int size);

        int retrieve (int size) { return retrieve(nullptr, size); }

        void clearAll ();

        void ensureWriteableBytes (size_t len);

        void appendWithoutSpaceEnsurance (const void* data, size_t len);

        void append (const void* data, size_t len);

        void appendInt32 (int32_t data);

        void appendChar (char data);

        void appendString (const std::string& data);

        int readFd (int fd, int& error);

    private:
        std::vector<char>  _buf;
        size_t             _read_pos;
        size_t             _write_pos;
};
