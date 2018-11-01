#include "Common.h"

#include "Buffer.h"
#include "SocketOps.h"
#include <cassert>

Buffer::Buffer () :
    _read_pos(0),
    _write_pos(0)
{

}

Buffer::~Buffer ()
{

}

int16_t
Buffer::peekInt16 () const
{
    assert(readableBytes() >= sizeof(int16_t));
    int16_t tmp = 0;
    ::memcpy(&tmp, peek(), sizeof(int16_t));
    return sockops::networkToHost16(tmp);
}

int16_t
Buffer::readInt16 ()
{
    int16_t ret = peekInt16();
    readSize(sizeof(ret));
    return ret;
}

int32_t
Buffer::peekInt32 () const
{
    assert(readableBytes() >= sizeof(int32_t));
    int32_t tmp = 0;
    ::memcpy(&tmp, peek(), sizeof(int32_t));
    return sockops::networkToHost32(tmp);
}

int32_t
Buffer::readInt32 ()
{
    int32_t ret = peekInt32();
    readSize(sizeof(ret));
    return ret;
}

int64_t
Buffer::peekInt64 () const
{
    assert(readableBytes() >= sizeof(int64_t));
    int64_t tmp = 0;
    ::memcpy(&tmp, peek(), sizeof(int64_t));
    return sockops::networkToHost64(tmp);
}

int64_t
Buffer::readInt64 ()
{
    int64_t ret = peekInt64();
    readSize(sizeof(ret));
    return ret;
}

std::string
Buffer::readString (size_t len)
{
    std::string ret(peek(), len);
    readSize(len);
    return ret;
}

void
Buffer::clearAll ()
{
    _write_pos = _read_pos = 0;
}

int
Buffer::retrieve (void* data, int size)
{
    int readable = _write_pos - _read_pos;
    if (size <= readable) {
        if (data) {
            ::memcpy(data, &_buf[_read_pos], size);
        }
        //printf("[Buffer] _read_pos: %d, data: %s\n", _read_pos, (char*)data);
        _read_pos += size;
        return size;
    } else {
        if (data) {
            ::memcpy(data, &_buf[_read_pos], readable);
        }
        //printf("[Buffer] _read_pos: %d, data: %s\n", _read_pos, (char*)data);
        _read_pos = _write_pos;
        return readable;
    }
}

void
Buffer::ensureWriteableBytes (size_t len)
{
    size_t writeable = _buf.size() - _write_pos;
    if (len > writeable) {
        if (len <= _read_pos + writeable) {
            if (_write_pos > _read_pos) {
                // FIXME
                ::memmove(_buf.data(), _buf.data() + _read_pos, len);
            }
            _write_pos -= _read_pos;
            _read_pos = 0;
        } else {
            //std::cout << "resize " << _buf.size() + len - writeable << " bytes" << std::endl;
            _buf.resize(_buf.size() + len - writeable);
        }
    }
}

void
Buffer::appendWithoutSpaceEnsurance (const void* data, size_t len)
{
    assert(_write_pos + len <= _buf.size());
    ::memcpy(_buf.data() + _write_pos, data, len);
    writeSize(len);
}

void
Buffer::append (const void* data, size_t len)
{
    ensureWriteableBytes(len);
    appendWithoutSpaceEnsurance(data, len);
}

void
Buffer::appendChar (char data)
{
    append(&data, sizeof(char));
}

void
Buffer::appendString (const std::string& str)
{
    append(str.data(), str.length());
}

void
Buffer::appendInt32 (int32_t data)
{
    append(&data, sizeof(int32_t));
}

int
Buffer::readFd (int fd, int& error)
{
    const int N = 65535;
    char buf[N];
    ssize_t n = ::read(fd, &buf, N);
    if (n >= 0) {
        append(buf, n);
        return n;
    } else {
        error = errno;
        return -1;
    }
}
