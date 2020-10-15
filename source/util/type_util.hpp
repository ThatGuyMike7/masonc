#ifndef $_MASONC_TYPE_UTIL_HPP_$
#define $_MASONC_TYPE_UTIL_HPP_$

#include <common.hpp>

#include <type_traits>

namespace masonc
{
    // Use semantically as a boolean type whenever "bool" does not work as expected,
    // for example "std::vector<bool>".
    using BOOL = u8;
    constexpr u8 TRUE = 1;
    constexpr u8 FALSE = 0;

    // "ref_t" is the same type as "value_t" if "value_t" is arithmetic, a pointer or a reference.
    // Otherwise, "ref_t" is "value_t&".
    template <typename value_t>
    using ref_t = std::conditional_t<std::is_arithmetic_v<value_t>,
                                     value_t,
                  std::conditional_t<std::is_pointer_v<value_t>,
                                     value_t,
                  std::conditional_t<std::is_reference_v<value_t>,
                                     value_t,
                                     value_t&>>>;

    template <typename value_t>
    struct const_ref_no_arithmetic
    {
        using type = const value_t&;
    };

    template <typename value_t>
    struct const_ref_no_arithmetic<value_t*>
    {
        using type = const value_t*;
    };

    template <typename value_t>
    struct const_ref_no_arithmetic<value_t&>
    {
        using type = const value_t&;
    };

    template <typename value_t>
    struct const_ref_arithmetic
    {
        using type = value_t;
    };

    template <typename value_t>
    struct const_ref
    {
        using type = std::conditional_t<std::is_pointer_v<value_t>,
                                        typename const_ref_no_arithmetic<value_t>::type,
                     std::conditional_t<std::is_reference_v<value_t>,
                                        typename const_ref_no_arithmetic<value_t>::type,
                     std::conditional_t<std::is_arithmetic_v<value_t>,
                                        typename const_ref_arithmetic<value_t>::type,
                                        typename const_ref_no_arithmetic<value_t>::type>>>;
    };

    // "const_ref_t" is the same as "const value_t" if "value_t" is a pointer or a reference.
    // If "value_t" is not a pointer nor a reference, but it is arithmetic, then
    // "const_ref_t" is the same type as "value_t".
    // Otherwise, "const_ref_t" is "const value_t&".
    template <typename value_t>
    using const_ref_t = typename const_ref<value_t>::type;
}

#endif