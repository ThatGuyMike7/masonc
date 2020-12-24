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

    template <typename value_t>
    struct ptr
    {
        using type = value_t*;
    };

    template <typename value_t>
    struct ptr<value_t&>
    {
        using type = value_t*;
    };

    template <typename value_t>
    struct ptr<value_t*>
    {
        using type = value_t*;
    };

    template <typename value_t>
    struct ptr<value_t* const>
    {
        using type = value_t* const;
    };

    // "ptr_t" is the same type as "value_t" if "value_t" is a pointer.
    // Otherwise, "ptr_t" is "value_t*".
    template <typename value_t>
    using ptr_t = typename ptr<value_t>::type;

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
    struct const_ref_no_arithmetic<value_t* const>
    {
        using type = const value_t* const;
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

    template <typename value_t>
    struct deref
    {
        using type = value_t;
    };

    template <typename value_t>
    struct deref<value_t*>
    {
        using type = value_t&;
    };

    template <typename value_t>
    struct deref<value_t* const>
    {
        using type = value_t&;
    };

    // If "value_t" is a pointer, it is turned into a reference.
    // Otherwise, "deref_t" is "value_t".
    template <typename value_t>
    using deref_t = typename deref<value_t>::type;

    // Turns a pointer value into a reference value.
    // Useful for template metaprogramming where
    // template parameters can be pointers or non-pointer types.
    template <typename value_t>
    constexpr deref_t<value_t> dereference(value_t value)
    {
        if constexpr (std::is_pointer_v<value_t>)
            return *value;
        else
            return value;
    }
}

#endif