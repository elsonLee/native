#pragma once

#include <string>
#include <cassert>
#include <cstring>

class Slice
{
    public:
        Slice() : _data(""), _size(0) {}

        Slice (const char* data, size_t size)
            : _data(data), _size(size) {}

        Slice (const std::string& str)
            : _data(str.data()), _size(str.size()) {}

        Slice (const char* cstr)
            : _data(cstr), _size(strlen(cstr)) {}

        Slice (const Slice&) = default;
        Slice& operator= (const Slice&) = default;

        const char* data () const { return _data; }

        size_t size () const { return _size; }

        bool empty () const { return _size == 0; }

        char operator[] (size_t n) const {
            assert(n < size());
            return _data[n];
        }

        void clear () { _data = ""; _size = 0; }

        std::string toString () const {
            return std::string(_data, _size);
        }

    private:
        const char* _data;
        size_t      _size;
};

inline bool
operator== (const Slice& lhs, const Slice& rhs)
{
    return ((lhs.size() == rhs.size()) &&
            (memcmp(lhs.data(), rhs.data(), lhs.size()) == 0));
}

inline bool
operator!= (const Slice& lhs, const Slice& rhs)
{
    return !(lhs == rhs);
}
