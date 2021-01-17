#ifndef $_MASONC_CSTRING_COLLECTION_BASIC_$
#define $_MASONC_CSTRING_COLLECTION_BASIC_$

#include <common.hpp>
#include <log.hpp>

#include <vector>
#include <string>
#include <optional>
#include <cstdlib>
#include <cstring>
#include <type_traits>
#include <new>
#include <limits>

namespace masonc
{
    // TODO: Implement shrinking on copy assignment to potentially save on memory.
    // "length_t" refers to the maximum length of a string.
    template <typename length_t>
    struct cstring_collection_basic
    {
        static_assert(std::is_arithmetic <length_t>::value,
            "cstring_collection_basic length_t is not arithmetic.");

    private:
        // Strings of characters ending with null terminators.
        char* buffer;

        u64 occupied_bytes;
        u64 current_buffer_size;
        u64 buffer_chunk_size;

        // Contains pointer offsets to elements in "buffer".
        std::vector<u64> lookup;

        // Keeps track of each string's length.
        std::vector<length_t> lengths;

        void memory_allocation_error()
        {
            log_error("\"cstring_collection_basic\" memory allocation error.");
            throw std::bad_alloc{};
        }

        // Grow to a multiple of "buffer_chunk_size" that is greater than "size".
        void grow_to_fit(u64 size)
        {
            if (size >= current_buffer_size) {
                do {
                    current_buffer_size += buffer_chunk_size;
                } while (size >= current_buffer_size);

                buffer = static_cast<char*>(std::realloc(buffer, current_buffer_size));
                if (buffer == nullptr)
                    memory_allocation_error();
            }
        }

    public:
        cstring_collection_basic(u64 buffer_chunk_size = 4096)
            : buffer(static_cast<char*>(std::malloc(buffer_chunk_size))),
              occupied_bytes(0),
              current_buffer_size(buffer_chunk_size),
              buffer_chunk_size(buffer_chunk_size),
              lookup(),
              lengths()
        {
            if (buffer == nullptr)
                memory_allocation_error();
        }

        ~cstring_collection_basic()
        {
            // Buffer can be nullptr if the cstring_collection was moved.
            if (buffer != nullptr)
                std::free(buffer);
        }

        cstring_collection_basic(const cstring_collection_basic& other)
            : buffer(static_cast<char*>(std::malloc(other.current_buffer_size))),
              occupied_bytes(other.occupied_bytes),
              current_buffer_size(other.current_buffer_size),
              buffer_chunk_size(other.buffer_chunk_size),
              lookup(other.lookup),
              lengths(other.lengths)
        {
            if (buffer == nullptr)
                memory_allocation_error();

            std::memcpy(buffer, other.buffer, other.occupied_bytes);
        }

        cstring_collection_basic& operator=(const cstring_collection_basic& other)
        {
            occupied_bytes = other.occupied_bytes;
            buffer_chunk_size = other.buffer_chunk_size;
            lookup = other.lookup;
            lengths = other.lengths;

            grow_to_fit(other.occupied_bytes);
            std::memcpy(buffer, other.buffer, other.occupied_bytes);

            return *this;
        }

        cstring_collection_basic(cstring_collection_basic&& other)
            : buffer(other.buffer),
              occupied_bytes(other.occupied_bytes),
              current_buffer_size(other.current_buffer_size),
              buffer_chunk_size(other.buffer_chunk_size),
              lookup(std::move(other.lookup)),
              lengths(std::move(other.lengths))
        {
            other.buffer = nullptr;
        }

        cstring_collection_basic& operator=(cstring_collection_basic&& other)
        {
            if (buffer != nullptr)
                std::free(buffer);

            buffer = other.buffer;
            other.buffer = nullptr;

            occupied_bytes = other.occupied_bytes;
            current_buffer_size = other.current_buffer_size;
            buffer_chunk_size = other.buffer_chunk_size;
            lookup = std::move(other.lookup);
            lengths = std::move(other.lengths);

            return *this;
        }

        // "length" is expected to not count the null terminator.
        // Returns the index of the string.
        u64 copy_back(const char* str, length_t length)
        {
            u64 occupied_bytes_after_copy = occupied_bytes + length + 1;
            grow_to_fit(occupied_bytes_after_copy);

            u64 index = lookup.size();
            lookup.push_back(occupied_bytes);
            lengths.push_back(length);

            std::memcpy(buffer + occupied_bytes, str, length + 1);
            occupied_bytes = occupied_bytes_after_copy;

            return index;
        }

        u64 copy_back(const std::string& str)
        {
            // Make sure conversion from "size_t" to "length_t" goes smooth in case the
            // maximum value of "length_t" is smaller than the maximum value of "size_t".
            assume(str.length() <= std::numeric_limits<length_t>::max());

            return copy_back(str.c_str(), static_cast<length_t>(str.length()));
        }

        const char* at(u64 index)
        {
            return buffer + lookup[index];
        }

        length_t length_at(u64 index)
        {
            return lengths[index];
        }

        u64 size()
        {
            return lookup.size();
        }

        // Perform a linear search to find a specific string in O(n) time
        // and return its index if the string is found.
        std::optional<u64> find(const char* str)
        {
            for (u64 i = 0; i < lookup.size(); i += 1)
                if (std::strcmp(str, at(i)) == 0)
                    return i;

            return std::nullopt;
        }
    };
}

#endif