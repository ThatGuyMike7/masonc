#ifndef $_MASONC_CSTRING_UTIL_HPP_$
#define $_MASONC_CSTRING_UTIL_HPP_$

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