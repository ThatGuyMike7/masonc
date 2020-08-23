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

namespace masonc
{
    // TODO: Implement shrinking on copy assignment to potentially save on memory.
    // "length_t" refers to the maximum length of a string that can be stored.
    template<typename length_t>
    struct cstring_collection_basic
    {
        static_assert(std::is_arithmetic<length_t>::value,
            "cstring_collection_basic length_t is not arithmetic.");

    private:
        // Items are composed of a "length_t" and
        // a sequence of characters ending with a null-terminator.
        char* buffer;

        // Contains buffer pointer offsets to the sequences of characters.
        std::vector<u64> lookup;

        u64 occupied_bytes;
        u64 current_buffer_size;
        u64 buffer_chunk_size;

        void memory_allocation_error_exit()
        {
            log_error("\"cstring_collection_basic\" memory allocation error.");
            std::exit(-1);
        }

        // Grow to a multiple of "buffer_chunk_size" that is greater or equal to "size".
        void grow_to_fit(u64 size)
        {
            if (size >= current_buffer_size) {
                do {
                    current_buffer_size += buffer_chunk_size;
                } while (size >= current_buffer_size);

                buffer = static_cast<char*>(
                    std::realloc(static_cast<void*>(buffer), current_buffer_size)
                );

                if (buffer == nullptr)
                    memory_allocation_error_exit();
            }
        }

    public:
        cstring_collection_basic(u64 buffer_chunk_size = 4096)
        {
            buffer = static_cast<char*>(std::malloc(buffer_chunk_size));
            if (buffer == nullptr)
                memory_allocation_error_exit();

            this->buffer_chunk_size = buffer_chunk_size;
            occupied_bytes = 0;
            current_buffer_size = buffer_chunk_size;
        }

        ~cstring_collection_basic()
        {
            // Buffer can be nullptr if the cstring_collection was moved.
            if (buffer != nullptr)
                std::free(buffer);
        }

        cstring_collection_basic(const cstring_collection_basic& other)
            : lookup(other.lookup),
              occupied_bytes(other.occupied_bytes),
              current_buffer_size(other.current_buffer_size),
              buffer_chunk_size(other.buffer_chunk_size)
        {
            std::memcpy(static_cast<void*>(buffer), static_cast<void*>(other.buffer), other.occupied_bytes);
        }

        cstring_collection_basic& operator=(const cstring_collection_basic& other)
        {
            buffer_chunk_size = other.buffer_chunk_size;
            lookup = other.lookup;
            occupied_bytes = other.occupied_bytes;

            grow_to_fit(other.occupied_bytes);
            std::memcpy(static_cast<void*>(buffer), static_cast<void*>(other.buffer), other.occupied_bytes);

            return *this;
        }

        cstring_collection_basic(cstring_collection_basic&& other)
            : lookup(other.lookup),
              occupied_bytes(other.occupied_bytes),
              current_buffer_size(other.current_buffer_size),
              buffer_chunk_size(other.buffer_chunk_size),
              buffer(other.buffer)
        {
            other.buffer = nullptr;
        }

        cstring_collection_basic& operator=(cstring_collection_basic&& other)
        {
            buffer_chunk_size = other.buffer_chunk_size;
            lookup = other.lookup;
            occupied_bytes = other.occupied_bytes;

            if (buffer != nullptr)
                std::free(buffer);

            buffer = other.buffer;
            other.buffer = nullptr;

            return *this;
        }

        // "length" is expected to not count the null terminator.
        // Returns the index of the string.
        u64 copy_back(const char* str, length_t length)
        {
            u64 item_size = sizeof(length_t) + length + 1;
            grow_to_fit(occupied_bytes + item_size);

            u64 index = lookup.size();
            lookup.push_back(occupied_bytes + sizeof(length_t));

            char* destination = buffer + occupied_bytes;
            std::memcpy(destination, &length, sizeof(length_t));

            destination += sizeof(length_t);
            std::memcpy(destination, str, length + 1);

            occupied_bytes += item_size;

            return index;
        }

        u64 copy_back(const std::string& str)
        {
            return copy_back(str.c_str(), str.length());
        }

        const char* at(u64 index)
        {
            return buffer + lookup[index];
        }

        length_t length_at(u64 index)
        {
            length_t length;
            std::memcpy(&length, buffer + lookup[index] - sizeof(length_t), sizeof(length_t));

            return length;
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