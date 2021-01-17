#ifndef MASONC_CSTRING_UTIL_HPP
#define MASONC_CSTRING_UTIL_HPP

namespace masonc
{
    struct cstring_hasher
    {
        size_t operator() (const char* str) const;
    };

    struct cstring_comparator_equal
    {
        bool operator() (const char* lhs, const char* rhs) const;
    };
}

#endif