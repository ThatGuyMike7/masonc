#include <cstring_util.hpp>

#include <cstring>
#include <string_view>
#include <functional>

namespace masonc
{
    size_t cstring_hasher::operator() (const char* str) const
    {
        return std::hash<std::string_view>()(std::string_view{ str });
    }

    bool cstring_comparator_equal::operator() (const char* lhs, const char* rhs) const
    {
        return std::strcmp(lhs, rhs) == 0;
    }
}