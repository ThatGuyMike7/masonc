#ifndef $_MASONC_CONTAINERS_HPP_$
#define $_MASONC_CONTAINERS_HPP_$

#include <cstring_util.hpp>
#include <cstring_collection_basic.hpp>

#include <unordered_set>
#include <unordered_map>

namespace masonc
{
    using cstring_collection = cstring_collection_basic<u16>;

    using cstring_unordered_set = std::unordered_set<const char*,
        cstring_hasher, cstring_comparator_equal, std::allocator<const char*>>;

    template <typename value_type>
    using cstring_unordered_map = std::unordered_map<const char* const, value_type,
        cstring_hasher, cstring_comparator_equal,
        std::allocator<std::pair<const char* const, value_type>>>;
}

#endif