#include <type_util.hpp>

#include <type_traits>
#include <string>

namespace masonc
{
    // Test if "ref_t" works as intended.
    static_assert(std::is_same_v<ref_t<int>, int>);
    static_assert(std::is_same_v<ref_t<int*>, int*>);
    static_assert(std::is_same_v<ref_t<int&>, int&>);

    static_assert(std::is_same_v<ref_t<std::string*>, std::string*>);
    static_assert(std::is_same_v<ref_t<std::string>, std::string&>);
}