#include <stack.h>

#include <stdio.h>
#include <string.h>

void (*stack_cont_error_handler_proc_ptr)(char*) = &stack_cont_error_handler_default;

void stack_cont_error_handler_default(char* message)
{
	printf("\n\nError in linear buffer: ");
	printf(message);
	printf("\n");
	getchar();
	exit(-1);
}

Stack stack_new(size_t element_size, size_t chunk_element_count)
{
	Stack s;
	s.chunk_size = element_size * chunk_element_count;
	s.size = s.chunk_size;
	s.element_size = element_size;
	s.length = 0;
	
	s.buffer = malloc(s.chunk_size);
	if(s.buffer == NULL)
		stack_cont_error_handler_proc_ptr("Memory exhausted");
		
	return s;
}

void stack_free(Stack* s)
{
	free(s->buffer);
}

void stack_grow(Stack* s)
{
	s->size += s->chunk_size;
	s->buffer = realloc(s->buffer, s->size);
	if(s->buffer == NULL)
		stack_cont_error_handler_proc_ptr("Memory exhausted");
}

void stack_push(Stack* s, void* element)
{
	size_t occupied_size = s->length * s->element_size;
	
	// Buffer is full, allocate larger one
	if(occupied_size == s->size)
		stack_grow(s);
	
	// Destination is one byte after everything occupied
	char* destination = s->buffer;
	destination += occupied_size;
	
	// Copy the element
	memcpy(destination, element, s->element_size);
	s->length += 1;
}

void stack_pop(Stack* s)
{
	s->length -= 1;
}

void* stack_at(Stack* s, size_t index)
{
	return (char*)s->buffer + index * s->element_size;
}