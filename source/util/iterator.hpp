#ifndef MASONC_ITERATOR_HPP
#define MASONC_ITERATOR_HPP

#include <common.hpp>

#include <iterator>
#include <type_traits>
#include <optional>

namespace masonc
{
    struct iterator_no_data { };

    template <typename element_t,
              typename iterator_data_t = iterator_no_data,
              typename size_t = u64>
    struct iterator
    {
    protected:
        const element_t* head;
        iterator_data_t data;

    public:
        // Iterator traits for STL compliance.
        using difference_type = element_t;
        using value_type = element_t;
        using pointer = const element_t*;
        using reference = const element_t&;
        using iterator_category = std::bidirectional_iterator_tag;

        template <typename T = iterator_data_t,
                  typename = std::enable_if_t<std::is_same_v<T, iterator_no_data>>>
        explicit iterator(pointer head)
            : head(head)
        { }

        template <typename T = iterator_data_t,
                  typename = std::enable_if_t<!std::is_same_v<T, iterator_no_data>>>
        explicit iterator(pointer head, T* data)
            : head(head), data(*data)
        { }

        iterator(const iterator& other)
        {
            if constexpr (std::is_same_v<iterator_data_t, iterator_no_data>) {
                head = other.head;
            }
            else {
                head = other.head;
                data = other.data;
            }
        }

        iterator& operator= (const iterator& other)
        {
            if constexpr (std::is_same_v<iterator_data_t, iterator_no_data>) {
                head = other.head;
            }
            else {
                head = other.head;
                data = other.data;
            }

            return *this;
        }

        bool operator== (const iterator& other) const
        {
            if constexpr (std::is_same_v<iterator_data_t, iterator_no_data>)
                return head == other.head;
            else
                return head == other.head && data == other.data;
        }

        bool operator!= (const iterator& other) const { return !(*this == other); }

        reference operator* () const { return *head; }

        // Pre-increment
        iterator& operator++ ()
        {
            head++;
            return *this;
        }

        // Pre-decrement
        iterator& operator-- ()
        {
            head--;
            return *this;
        }

        // Post-increment
        iterator operator++ (int)
        {
            iterator return_value = *this;

            // Do the actual increment.
            ++(*this);

            return return_value;
        }

        // Post-decrement
        iterator operator-- (int)
        {
            iterator return_value = *this;

            // Do the actual decrement.
            --(*this);

            return return_value;
        }

        // Binary addition compound assignment
        iterator& operator+= (size_t other)
        {
            head += other;

            return *this;
        }

        // Binary addition
        friend iterator operator+ (iterator lhs, size_t rhs)
        {
            // Call binary addition compound assignment.
            lhs += rhs;

            return lhs;
        }

        // Binary subtraction compound assignment
        iterator& operator-= (size_t other)
        {
            head -= other;

            return *this;
        }

        // Binary subtraction
        friend iterator operator- (iterator lhs, size_t rhs)
        {
            // Call binary subtraction compound assignment.
            lhs -= rhs;

            return lhs;
        }
    };

    template <typename element_t,
              template <typename> typename iterator_t = iterator>
    struct iterable
    {
        virtual iterator_t<element_t> begin() = 0;
        virtual iterator_t<element_t> end() = 0;
    };
}

#endif