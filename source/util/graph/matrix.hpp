#ifndef $_MASONC_MATRIX_HPP_$
#define $_MASONC_MATRIX_HPP_$

#include <common.hpp>
#include <iterator.hpp>

namespace masonc
{
    // Sparse Matrices:
    // https://www5.in.tum.de/pub/heinecke08.pdf

    template <typename element_t,
              template <typename> typename iterator_t = iterator,
              typename size_t = u64>
    struct matrix : public iterable<element_t, iterator_t>
    {
        matrix(size_t row_count, size_t column_count);

        virtual element_t& operator[] (size_t row, size_t column) = 0;
        virtual const element_t& operator[] (size_t row, size_t column) const = 0;

        virtual size_t row_count() const = 0;
        virtual size_t column_count() const = 0;
    };

    struct matrix_row_major_iterator_data
    {

    };

    template <typename element_t>
    struct matrix_row_major_iterator : public iterator<element_t>
    {

    };

    template <typename element_t,
              typename size_t = u64>
    struct matrix_row_major : public matrix<element_t, matrix_row_major_iterator, size_t>
    {
    private:
        element_t* buffer;

    public:
        matrix_row_major(size_t row_count, size_t column_count)
        {
            buffer = new element_t[row_count * column_count];
        }

        ~matrix_row_major()
        {
            delete[] buffer;
        }

        // TODO: Implement copy ctor, copy assignment op, move ctor, move assignment op.

        // Matrix overrides
        element_t& operator[] (size_t row, size_t column) override
        {

        }

        const element_t& operator[] (size_t row, size_t column) const override
        { }

        size_t row_count() const override
        { }
        size_t column_count() const override
        { }

        // Iterable overrides
        matrix_row_major_iterator<element_t> begin() override
        { }

        matrix_row_major_iterator<element_t> end() override
        { }
    };
}

#endif