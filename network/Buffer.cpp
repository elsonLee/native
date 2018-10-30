#include "Common.h"

#include "Buffer.h"
#include "SocketOps.h"
#include <cassert>

Buffer::Buffer () :
    _readPos(0),
    _writePos(0)
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
    _readPos += sizeof(ret);
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
    _readPos += sizeof(ret);
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
    _readPos += sizeof(ret);
    return ret;
}

void
Buffer::clearAll ()
{
    _writePos = _readPos = 0;
}

int
Buffer::retrieve (void* data, int size)
{
    int readable = _writePos - _readPos;
    if (size <= readable) {
        if (data) {
            ::memcpy(data, &_buf[_readPos], size);
        }
        //printf("[Buffer] _readPos: %d, data: %s\n", _readPos, (char*)data);
        _readPos += size;
        return size;
    } else {
        if (data) {
            ::memcpy(data, &_buf[_readPos], readable);
        }
        //printf("[Buffer] _readPos: %d, data: %s\n", _readPos, (char*)data);
        _readPos = _writePos;
        return readable;
    }
}


int
Buffer::append (const void* data, int size)
{
    int writeable = _buf.size() - _writePos;
    if (size <= writeable) {
        ::memcpy(&_buf[_writePos], data, size);
        _writePos += size;
        return size;
    } else {
        if (size <= _readPos + writeable) {
            if (_writePos > _readPos) {
                // FIXME
                ::memmove(_buf.data(), _buf.data() + _readPos, size);
            }
            _writePos -= _readPos;
            _readPos = 0;
        } else {
            //std::cout << "resize " << _buf.size() + size - writeable << " bytes" << std::endl;
            _buf.resize(_buf.size() + size - writeable);
        }
        return append(data, size);
    }
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
