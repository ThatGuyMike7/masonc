#ifndef $_MASONC_MATRIX_HPP_$
#define $_MASONC_MATRIX_HPP_$

#include <common.hpp>
#include <iterator.hpp>
#include <log.hpp>

#include <cstdlib>
#include <new>

namespace masonc
{
    template <typename element_t,
              template <typename> typename iterator_t = iterator,
              typename size_t = u64>
    struct matrix_basic : public iterable<element_t, iterator_t>
    {
        matrix_basic(size_t row_count, size_t column_count);

        virtual element_t& operator[] (size_t row, size_t column) = 0;
        virtual const element_t& operator[] (size_t row, size_t column) const = 0;

        virtual size_t row_count() const = 0;
        virtual size_t column_count() const = 0;
    };

    template <typename element_t>
    struct matrix_row_major_iterator : public iterator<element_t>
    { };

    // TODO: Implement tests for "matrix_row_major".
    template <typename element_t,
              typename size_t = u64>
    struct matrix_row_major : public matrix_basic<element_t, matrix_row_major_iterator, size_t>
    {
    private:
        element_t* buffer;
        size_t m_row_count, m_column_count;

        size_t index(size_t row, size_t column) const
        {
            return row * m_row_count + column;
        }

    public:
        matrix_row_major(size_t row_count, size_t column_count)
            : m_row_count(row_count),
              m_column_count(column_count)
        {
            size_t element_count = m_row_count * m_column_count;

            buffer = static_cast<element_t*>(std::malloc(sizeof(element_t) * element_count));

            if (buffer == nullptr) {
                log_error("Unable to allocate memory for \"matrix_row_major\".");
                throw std::bad_alloc{};
            }

            for (size_t i = 0; i < element_count; i += 1) {
                new(buffer + i) element_t;
            }
        }

        ~matrix_row_major()
        {
            for (size_t i = 0; i < m_row_count * m_column_count; i += 1) {
                element_t* current_element = buffer + i;
                current_element->~element_t();
            }

            std::free(buffer);
        }

        matrix_row_major(const matrix_row_major& other)
        {
            size_t element_count = m_row_count * m_column_count;
            buffer = static_cast<element_t*>(std::malloc(sizeof(element_t) * element_count));

            if (buffer == nullptr) {
                log_error("Unable to allocate memory for \"matrix_row_major\".");
                throw std::bad_alloc{};
            }

            std::copy(other.begin(), other.end(), this->begin());
        }

        matrix_row_major& operator= (const matrix_row_major& other)
        {
            size_t element_count = m_row_count * m_column_count;
            size_t other_element_count = other.m_row_count * other.m_column_count;

            for (size_t i = 0; i < element_count; i += 1) {
                element_t* current_element = buffer + i;
                current_element->~element_t();
            }

            buffer = std::realloc(buffer, sizeof(element_t) * other_element_count);

            if (buffer == nullptr) {
                log_error("Unable to allocate memory for \"matrix_row_major\".");
                throw std::bad_alloc{};
            }

            std::copy(other.begin(), other.end(), this->begin());
        }

        matrix_row_major(matrix_row_major&& other)
            : buffer(other.buffer),
              m_row_count(other.m_row_count),
              m_column_count(other.m_column_count)
        {
            other.buffer = nullptr;
        }

        matrix_row_major& operator= (matrix_row_major&& other)
        {
            if (buffer != nullptr)
                std::free(buffer);

            buffer = other.buffer;
            other.buffer = nullptr;

            m_row_count = other.m_row_count;
            m_column_count = other.m_column_count;

            return *this;
        }

        // Matrix overrides
        element_t& operator[] (size_t row, size_t column) override
        {
            return buffer[index(row, column)];
        }

        const element_t& operator[] (size_t row, size_t column) const override
        {
            return buffer[index(row, column)];
        }

        size_t row_count() const override
        {
            return m_row_count;
        }

        size_t column_count() const override
        {
            return m_column_count;
        }

        // Iterable overrides
        matrix_row_major_iterator begin() override
        {
            return matrix_row_major_iterator<element_t>{ buffer };
        }

        matrix_row_major_iterator end() override
        {
            return matrix_row_major_iterator<element_t>{ buffer + index(m_row_count, m_column_count) };
        }
    };
}

#endif