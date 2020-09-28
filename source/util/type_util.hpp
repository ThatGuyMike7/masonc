#ifndef $_MASONC_TYPE_UTIL_HPP_$
#define $_MASONC_TYPE_UTIL_HPP_$

#include <common.hpp>

#include <type_traits>

namespace masonc
{
   // "ref_t" is the same type as "value_t" if "value_t" is arithmetic, a pointer or a reference.
   // Otherwise, "ref_t" is a reference type.
   template <typename value_t>
   using ref_t = std::conditional_t<std::is_arithmetic_v<value_t>,
                                    value_t,
                 std::conditional_t<std::is_pointer_v<value_t>,
                                    value_t,
                 std::conditional_t<std::is_reference_v<value_t>,
                                    value_t,
                                    value_t&>>>;

    // Use semantically as a boolean type whenever "bool" does not work as expected,
    // for example "std::vector<bool>".
    using BOOL = u8;
    constexpr u8 TRUE = 1;
    constexpr u8 FALSE = 0;
}

#endif