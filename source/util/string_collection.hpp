#ifndef $_MASONC_STRING_COLLECTION_$
#define $_MASONC_STRING_COLLECTION_$

#include <common.hpp>
#include <vector>
#include <string>

namespace masonc
{
    // In DEBUG mode, `string_collection` is about 12% faster than `std::vector<std::string>`
    // when iterating over a large amount of data contiguously and
    // about 90% faster when appending a lot of individual elements in a loop.
    // With optimizations turned on, the iteration difference becomes marginal (around 3%) but
    // it's still a bit faster, while appending is about 42% faster.
	struct string_collection
	{
        string_collection(u64 buffer_chunk_size = 4096);
        ~string_collection();

        // 'length' is expected to not count the null terminator
        void add(const char* str, u64 length);
        void add(const std::string& str);

        char* get(u64 index);
        u64 length_at(u64 index) const;
        u64 count() const;

    private:

        // Items are composed of a `u64` and a sequence of characters ending with a null-terminator
        char* buffer;

        // Contains buffer pointer offsets to the sequences of characters
        std::vector<u64> lookup;

        u64 item_count;
        u64 occupied_bytes;
        u64 current_buffer_size;
        u64 buffer_chunk_size;
	};
}

#endif