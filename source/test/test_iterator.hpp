#ifndef $_MASONC_TEST_ITERATOR_HPP_$
#define $_MASONC_TEST_ITERATOR_HPP_$

#include <iterator.hpp>

namespace masonc::test::iterator
{
    struct test_container : public iterable<int>
    {
    private:
        int* data;
        int count;

    public:
        test_container(int count);
        ~test_container();

        masonc::iterator<int> begin() override;
        masonc::iterator<int> end() override;
    };

    void test_forward_iteration();
    void test_reverse_iteration();
    void test_STL_algorithms();
}

#endif