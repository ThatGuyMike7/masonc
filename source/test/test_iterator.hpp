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

    struct test_iterator_data
    {
        int step;

        bool operator== (const test_iterator_data& other) const
        {
            return step == other.step;
        }
    };

    template <typename element_t>
    struct test_iterator : public masonc::iterator<element_t, test_iterator_data>
    {
        explicit test_iterator(const element_t* head, test_iterator_data* data)
            : masonc::iterator<element_t, test_iterator_data>(head, data)
        { }

        // Pre-increment
        test_iterator& operator++()
        {
            head += data.step;
            return *this;
        }

        // Pre-decrement
        test_iterator& operator--()
        {
            head -= data.step;
            return *this;
        }

        // Post-increment
        test_iterator operator++(int)
        {
            test_iterator return_value = *this;

            // Do the actual increment.
            ++(*this);

            return return_value;
        }

        // Post-decrement
        test_iterator operator--(int)
        {
            test_iterator return_value = *this;

            // Do the actual decrement.
            --(*this);

            return return_value;
        }
    };

    struct test_container_2 : public iterable<int, test_iterator>
    {
    private:
        int* data;
        int count;
        int step;

    public:
        // "count" has to be a multiple of "step" for iteration to work.
        test_container_2(int count, int step);
        ~test_container_2();

        test_iterator<int> begin() override;
        test_iterator<int> end() override;
    };

    void test_forward_iteration();
    void test_reverse_iteration();
    void test_STL_algorithms();
    void test_iterator_data_iteration();
}

#endif