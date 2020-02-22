// smap = stack map or static map, because pairs cannot be removed
// best used for small data sets as find performs a naive linear search

#ifndef _MASON_SMAP_H_
#define _MASON_SMAP_H_

#include <stack.h>

typedef struct SMap
{
	Stack keys;
	Stack values;
	int (*comparator)(Stack*, void*);
	
} SMap;

// Allocation and initialization.
// The comparator function is expected to find an element casted to the specific type of the stack and return the index of that element.
// If not found, it is expected to return a negative value.
extern SMap smap_new(size_t key_element_size, size_t value_element_size, size_t chunk_element_count, int (*smap_comparator)(Stack*, void*));

// Free all your elements first if they contain any owning pointers
extern void smap_free(SMap* map);

// Copy a key-value pair into the map
extern void smap_push(SMap* map, void* key, void* value);

// Get a value by key. If the key does not exist, return a NULL-pointer.
extern void* smap_get(SMap* map, void* key);

#endif