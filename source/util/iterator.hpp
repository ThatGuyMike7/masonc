#ifndef $_MASONC_ITERATOR_HPP_$
#define $_MASONC_ITERATOR_HPP_$

#include <common.hpp>

#include <iterator>
#include <type_traits>
#include <optional>

namespace masonc
{
    struct iterator_no_data { };

    template <typename element_t,
              typename iterator_data = iterator_no_data>
    struct iterator
    {
    protected:
        const element_t* head;
        const iterator_data data;

    public:
        // Iterator traits for STL compliance.
        using difference_type = element_t;
        using value_type = element_t;
        using pointer = const element_t*;
        using reference = const element_t&;
        using iterator_category = std::bidirectional_iterator_tag;

        // "iterator_data" if "iterator_data" is not "iterator_no_data", otherwise no type.
        using optional_iterator_data_t = std::enable_if_t<
                                            !std::is_same_v<iterator_data, iterator_no_data>,
                                            iterator_data>;

        explicit iterator(pointer head, optional_iterator_data_t data)
            : head(head), data(data)
        { }

        bool operator== (iterator other) const { return head == other.head; }
        bool operator!= (iterator other) const { return !(*this == other); }

        reference operator* () const { return *head; }

        // Pre-increment
        virtual iterator& operator++()
        {
            head++;
            return *this;
        }

        // Pre-decrement
        virtual iterator operator--()
        {
            head--;
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

        // Post-decrement
        iterator operator--(int)
        {
            iterator return_value = *this;

            // Do the actual decrement.
            --(*this);

            return return_value;
        }
    };

    template <typename element_t,
              template <typename> typename iterator_t = iterator>
    struct iterable
    {
        static_assert(std::is_base_of_v<iterator<element_t>, iterator_t<element_t>>);

        virtual iterator_t<element_t> begin() = 0;
        virtual iterator_t<element_t> end() = 0;
    };
}

#endif