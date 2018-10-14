#include "Common.h"

#include "Buffer.h"

Buffer::Buffer () :
    _readPos(0),
    _writePos(0)
{

}


Buffer::~Buffer ()
{

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
        //printf("_writePos: %d, data: %s\n", _writePos, data);
        _writePos += size;
        return size;
    } else {
        if (size <= _readPos + writeable) {
            if (_writePos > _readPos) {
                std::swap(_buf.at(_readPos), _buf.at(0));
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
