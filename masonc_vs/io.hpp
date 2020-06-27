#ifndef $_MASONC_IO_HPP_$
#define $_MASONC_IO_HPP_$

#include "common.hpp"

namespace masonc
{
	// Read a file into a buffer. If 'terminator_index' is not nullptr, it will be set to the index of the null terminator (last byte in array).
	// std::free() has to be called on the buffer once it is not used anymore.
	result<char*> file_read(const char* path, const u64 block_size = 1024, u64* terminator_index = nullptr);
}

#endif