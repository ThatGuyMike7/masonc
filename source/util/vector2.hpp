#ifndef MASONC_VECTOR2_HPP
#define MASONC_VECTOR2_HPP

#include <common.hpp>

#include <vector>

namespace masonc
{
    template <typename value_t, value_t zero_value = value_t{}>
    struct vector2
    {
        vector2(u64 column_count)
            : m_column_count(column_count), m_row_count(0)
        {
            m_buffer.reserve(m_column_count);
        }

        // The vector must have no elements.
        vector2(std::vector<value_t>&& buffer, u64 column_count)
            : m_column_count(column_count), m_row_count(0)
        {
            assume(m_buffer.size() == 0);

            m_buffer = std::vector<value_t>(buffer);
            m_buffer.reserve(column_count);
        }

        // The vector must have no elements.
        vector2(const std::vector<value_t>& buffer, u64 column_count)
            : m_column_count(column_count), m_row_count(0)
        {
            assume(m_buffer.size() == 0);

            m_buffer = buffer;
            m_buffer.reserve(column_count);
        }

        // Fills a new row with "zero_value" and returns a pointer to the first element in the new row.
        value_t* new_row()
        {
            m_row_count += 1;
            return &*m_buffer.insert(m_buffer.end(), m_column_count, zero_value);
        }

        // Fills a new row with "fill_value" and returns a pointer to the first element in the new row.
        value_t* new_row(const_ref_t<value_t> fill_value)
        {
            m_row_count += 1;
            return &*m_buffer.insert(m_buffer.end(), m_column_count, fill_value);
        }

        // Returns a pointer to the first element in the row.
        // Make sure the row actually exists.
        value_t* row(u64 row_number) const
        {
            return &m_buffer[row_number * m_column_count];
        }

        /*
        const std::vector<value_t>& buffer() const
        {
            return m_buffer;
        }
        */

        u64 column_count() const
        {
            return m_column_count;
        }

        u64 row_count() const
        {
            return m_row_count;
        }

    private:
        std::vector<value_t> m_buffer;
        u64 m_column_count;
        u64 m_row_count;
    };
}

#endif