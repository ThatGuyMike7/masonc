#ifndef MASONC_CONTAINERS_HPP
#define MASONC_CONTAINERS_HPP

#include <cstring_util.hpp>
#include <cstring_collection_basic.hpp>

#include <robin_hood.hpp>

namespace masonc
{
    using cstring_collection = cstring_collection_basic<u16>;

    using cstring_unordered_set = robin_hood::unordered_set<const char*,
        cstring_hasher, cstring_comparator_equal>;

    template <typename value_type>
    using cstring_unordered_map = robin_hood::unordered_map<const char* const, value_type,
        cstring_hasher, cstring_comparator_equal>;
}

#endif