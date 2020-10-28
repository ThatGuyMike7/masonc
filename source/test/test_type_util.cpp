#include <test_type_util.hpp>

#include <type_traits>
#include <string>

namespace masonc::test::type_util
{
    // Test if "ptr_t" works as intended.
    static_assert(std::is_same_v<ptr_t<int>, int*>);
    static_assert(std::is_same_v<ptr_t<int*>, int*>);
    static_assert(std::is_same_v<ptr_t<int&>, int*>);

    static_assert(std::is_same_v<ptr_t<const int>, const int*>);
    static_assert(std::is_same_v<ptr_t<const int*>, const int*>);
    static_assert(std::is_same_v<ptr_t<const int* const>, const int* const>);
    static_assert(std::is_same_v<ptr_t<int* const>, int* const>);
    static_assert(std::is_same_v<ptr_t<const int&>, const int*>);

    // Test if "ref_t" works as intended.
    static_assert(std::is_same_v<ref_t<int>, int>);
    static_assert(std::is_same_v<ref_t<int*>, int*>);
    static_assert(std::is_same_v<ref_t<int&>, int&>);
    static_assert(std::is_same_v<ref_t<std::string*>, std::string*>);
    static_assert(std::is_same_v<ref_t<std::string>, std::string&>);

    static_assert(std::is_same_v<ref_t<const int>, const int>);
    static_assert(std::is_same_v<ref_t<const int*>, const int*>);
    static_assert(std::is_same_v<ref_t<const int* const>, const int* const>);
    static_assert(std::is_same_v<ref_t<int* const>, int* const>);
    static_assert(std::is_same_v<ref_t<const int&>, const int&>);


    // Test if "const_ref_t" works as intended.
    static_assert(std::is_same_v<const_ref_t<int>, int>);
    static_assert(std::is_same_v<const_ref_t<int*>, const int*>);
    static_assert(std::is_same_v<const_ref_t<int&>, const int&>);
    static_assert(std::is_same_v<const_ref_t<std::string*>, const std::string*>);
    static_assert(std::is_same_v<const_ref_t<std::string>, const std::string&>);

    static_assert(std::is_same_v<const_ref_t<const int>, const int>);
    static_assert(std::is_same_v<const_ref_t<const int*>, const int*>);
    static_assert(std::is_same_v<const_ref_t<const int* const>, const int* const>);
    static_assert(std::is_same_v<const_ref_t<int* const>, const int* const>);
    static_assert(std::is_same_v<const_ref_t<const int&>, const int&>);

    // Test if "deref_t" works as intended.
    static_assert(std::is_same_v<deref_t<int>, int>);
    static_assert(std::is_same_v<deref_t<int*>, int&>);
    static_assert(std::is_same_v<deref_t<int&>, int&>);
    static_assert(std::is_same_v<deref_t<const int>, const int>);
    static_assert(std::is_same_v<deref_t<const int*>, const int&>);
    static_assert(std::is_same_v<deref_t<const int* const>, const int&>);
    static_assert(std::is_same_v<deref_t<int* const>, int&>);
    static_assert(std::is_same_v<deref_t<const int&>, const int&>);
}