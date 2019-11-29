#ifndef _STRING_BUILDER_H_
#define _STRING_BUILDER_H_

#include <string.h>

typedef struct StringBuilder
{
	// Initial buffer size
	// Reallocation grows the buffer by `buffer_size` bytes
	size_t chunk;
	
	// Size of the buffer
	size_t size;
	
	// Count of characters in the string that are not empty
	size_t length;
	
	char* buffer;
} StringBuilder;

// Construct a new string builder (yes, this includes memory allocation) and empty the string (fill it with '\0')
extern StringBuilder sb_new(size_t chunk);

// Free the string builder's buffer
extern void sb_free(StringBuilder* sb);

// Empty a string by setting all its characters to '\0'
extern void sb_empty(StringBuilder* sb);

// Cut the string so it ends with a single '\0'
// Involves memory allocation and returns a cut copy of the source string
extern char* sb_cut(StringBuilder* sb);

// Add a char at the end of the string
extern void sb_push_char(StringBuilder* sb, char c);

// Set the last char char in the string to '\0'
// Undefined behaviour if length < 1 so check for that if necessary
extern void sb_pop_char(StringBuilder* sb);

#endif