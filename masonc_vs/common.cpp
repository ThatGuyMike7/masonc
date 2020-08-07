#include "common.hpp"

#include <limits>

namespace masonc
{
	// Assert size of integer types.
	static_assert(sizeof(s8) == 1 && sizeof(s16) == 2 && sizeof(s32) == 4 && sizeof(s64) == 8 &&
				  sizeof(u8) == 1 && sizeof(u16) == 2 && sizeof(u32) == 4 && sizeof(u64) == 8, 
				  "Integer types do not have the expected size");
	
	// Assert size of floating point types.
	static_assert(sizeof(f32) == 4 && sizeof(f64) == 8,
				  "Floating point types do not have the expected size");
	
	// Assert that "char" is signed.
	static_assert(std::numeric_limits<char>::is_signed);
	
	// Checks
	static_assert(UINTMAX_MAX <= UINT64_MAX, "UINTMAX_MAX (maximum value of uintmax_t)"
		"is greater than UINT64_MAX (maximum value of u64/uint64_t)");
	static_assert(SIZE_MAX <= UINT64_MAX, "'SIZE_MAX' (maximum value of size_t)"
		"is greater than UINT64_MAX (maximum value of u64/uint64_t)");

	void assume(bool statement, const char* msg, int code)
	{
		#if defined(MASONC_DEBUG)
		if (statement)
			return;

		std::cout << "Assume failed: " << msg << std::endl;
		std::cout << "Press any key to exit with code " << code << std::endl;
		std::getchar();
		exit(code);
		#endif
	}
}