#include <smap.h>

SMap smap_new(size_t key_element_size, size_t value_element_size, size_t chunk_element_count, int (*smap_comparator)(Stack*, void*))
{
	SMap map;
	map.keys = stack_new(key_element_size, chunk_element_count);
	map.values = stack_new(key_element_size, chunk_element_count);
	map.comparator = smap_comparator;
	return map;
}

void smap_free(SMap* map)
{
	stack_free(&map->keys);
	stack_free(&map->values);
}

void smap_push(SMap* map, void* key, void* value)
{
	stack_push(&map->keys, key);
	stack_push(&map->values, value);
}

void* smap_get(SMap* map, void* key)
{
	int index = map->comparator(&map->keys, key);
	if(index < 0)
		return (void*)0;
	
	return stack_at(&map->values, (size_t)index);
}