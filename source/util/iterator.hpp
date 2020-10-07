#ifndef $_MASONC_ITERATOR_HPP_$
#define $_MASONC_ITERATOR_HPP_$

#include <common.hpp>

#include <iterator>

namespace masonc
{
    template <typename element_t>
    struct iterator
    {
    private:
        const element_t* head;

    public:
        // Iterator traits for STL compliance.
        using difference_type = element_t;
        using value_type = element_t;
        using pointer = const element_t*;
        using reference = const element_t&;
        using iterator_category = std::bidirectional_iterator_tag;

        explicit iterator(pointer head)
            : head(head)
        { }

        // Pre-increment
        iterator& operator++()
        {
            head++;
            return *this;
        }

        // Post-increment
        iterator operator++(int)
        {
            iterator return_value = *this;

            // Do the actual increment.
            ++(*this);

            return return_value;
        }

        // Pre-decrement
        iterator operator--()
        {
            head--;
            return *this;
        }

        // Post-decrement
        iterator operator--(int)
        {
            iterator return_value = *this;

            // Do the actual decrement.
            --(*this);

            return return_value;
        }

        bool operator== (iterator other) const { return head == other.head; }
        bool operator!= (iterator other) const { return !(*this == other); }

        reference operator* () const { return *head; }
    };

    template <typename element_t>
    struct iterable
    {
        virtual iterator<element_t> begin() = 0;
        virtual iterator<element_t> end() = 0;
    };
}

#endif