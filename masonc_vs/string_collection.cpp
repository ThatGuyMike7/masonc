#include "string_collection.hpp"

namespace masonc
{
    void string_collection::add(const char* str, u64 length)
    {
        u64 item_size = sizeof(u64) + length + 1;
        if (current_buffer_size < occupied_bytes + item_size)
        {
            current_buffer_size += buffer_chunk_size;
            buffer = static_cast<char*>(std::realloc(buffer, current_buffer_size));
        }
        
        lookup.push_back(occupied_bytes + sizeof(u64));
        
        char* destination = buffer + occupied_bytes;
        std::memcpy(destination, &length, sizeof(u64));
        
        destination += sizeof(u64);
        std::memcpy(destination, str, length + 1);
        
        occupied_bytes += item_size;
        item_count += 1;
    }

    void string_collection::add(const std::string& str)
    {
        add(str.c_str(), str.length());
    }
    
    char* string_collection::get(u64 index)
    {
        return buffer + lookup[index];
    }
    
    u64 string_collection::length_at(u64 index) const
    {
        u64 length;
        memcpy(&length, buffer + lookup[index] - sizeof(u64), sizeof(u64));
        return length;
    }

    u64 string_collection::count() const
    {
        return item_count;
    }
}