#ifndef $_MASONC_MATRIX_HPP_$
#define $_MASONC_MATRIX_HPP_$

#include <common.hpp>

namespace masonc
{
    // Sparse Matrices:
    // https://www5.in.tum.de/pub/heinecke08.pdf

    template <typename element_t,
              typename size_t = u64>
    struct matrix
    {
    private:

    public:
        struct iterator
        {
        private:

        };

        matrix(size_t row_count, size_t column_count);

        virtual element_t& operator[] (size_t row, size_t column) = 0;
        virtual const element_t& operator[] (size_t row, size_t column) const = 0;

        virtual size_t row_count() const = 0;
        virtual size_t column_count() const = 0;
    };
}

#endif