// The contents of this file is based on boost's "hash_combine" function
// (see "masonc/LICENSE-THIRD-PARTY.txt").

#ifndef MASONC_BOOST_HASH_COMBINE_HPP
#define MASONC_BOOST_HASH_COMBINE_HPP

#include <common.hpp>

#include <functional>

namespace boost
{
    template <typename T, typename... Rest>
    void hash_combine(masonc::u64* seed, const T& v, const Rest&... rest)
    {
        seed ^= std::hash<T>{}(v) + 0x9e3779b9 + ((*seed) << 6) + ((*seed) >> 2);
        (hash_combine(seed, rest), ...);
    }
}

#endif