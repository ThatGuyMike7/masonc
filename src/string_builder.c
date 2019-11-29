#include <string_builder.h>

#include <common.h>

#include <stdlib.h>
#include <stdio.h>

StringBuilder sb_new(size_t chunk)
{
	StringBuilder sb;
	sb.chunk = chunk;
	sb.size = chunk;
	//sb.length = 0;  <-- Called in "sb_empty()"
	
	sb.buffer = (char*)malloc(chunk);
	if(sb.buffer == NULL)
		quit("StringBuilder buffer memory exhausted");
	
	sb_empty(&sb);
	return sb;
}

void sb_free(StringBuilder* sb)
{
	free(sb->buffer);
}

void sb_empty(StringBuilder* sb)
{
	sb->length = 0;
	for(size_t i = 0; i < sb->size; i++)
		sb->buffer[i] = '\0';
}

char* sb_cut(StringBuilder* sb)
{
	size_t size = sb->length + 1;
	
	char* cut = malloc(size);
	if(cut == NULL)
		quit("StringBuilder buffer memory exhausted");
		
	memcpy(cut, sb->buffer, size);
	if(cut == NULL)
		quit("StringBuilder memcpy failed");
	
	return cut;
}

void sb_push_char(StringBuilder* sb, char c)
{
	// Buffer is too small, allocate larger one
	// At least one '\0' is guaranteed at the end of string
	if(sb->length == sb->size - 1)
	{
		size_t i = sb->size;
		sb->size += sb->chunk;
		
		sb->buffer = realloc(sb->buffer, sb->size);
		if(sb->buffer == NULL)
			quit("StringBuilder buffer memory exhausted");
		
		// Set all newly allocated characters to '\0'
		while(i < sb->size)
		{
			sb->buffer[i] = '\0';
			i++;
		}
	}
	
	sb->buffer[sb->length] = c;
	sb->length += 1;
}

void sb_pop_char(StringBuilder* sb)
{
	sb->buffer[sb->length - 1] = '\0';
}