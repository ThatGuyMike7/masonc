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
	
	s.swap_buffer = malloc(s.element_size);
	if(s.swap_buffer == NULL)
		stack_cont_error_handler_proc_ptr("Memory exhausted");
	
	return s;
}

void stack_free(Stack* s)
{
	free(s->buffer);
	free(s->swap_buffer);
}

void stack_grow(Stack* s, size_t chunks)
{
	s->size += s->chunk_size * chunks;
	s->buffer = realloc(s->buffer, s->size);
	if(s->buffer == NULL)
		stack_cont_error_handler_proc_ptr("Memory exhausted");
}

void stack_push(Stack* s, void* element)
{
	size_t occupied_size = s->length * s->element_size;
	
	// Buffer is full, grow by 1 chunk
	if(occupied_size == s->size)
		stack_grow(s, 1);
	
	// Destination is one byte after everything occupied
	char* destination = s->buffer;
	destination += occupied_size;
	
	// Copy the element
	memcpy(destination, element, s->element_size);
	s->length += 1;
}

void stack_pushs(Stack* dest_stack, Stack* src_stack)
{
	size_t occupied_size = dest_stack->length * dest_stack->element_size;
	
	// Minimum size needed to fit `src_stack` onto `dest_stack`
	size_t min_size = (dest_stack->length + src_stack->length) * dest_stack->element_size;
	
	// Buffer is too small, allocate one that is large enough
	if(dest_stack->size < min_size)
	{
		size_t chunks = (min_size - dest_stack->size) / dest_stack->chunk_size;
		stack_grow(dest_stack, chunks);
	}
	
	// Destination is one byte after everything occupied
	char* destination = dest_stack->buffer;
	destination += occupied_size;
	
	// Copy the stack
	memcpy(destination, src_stack->buffer, src_stack->length * dest_stack->element_size);
}

void stack_pop(Stack* s)
{
	s->length -= 1;
}

void* stack_at(Stack* s, size_t index)
{
	return (char*)s->buffer + index * s->element_size;
}

void stack_swap(Stack* s, size_t i1, size_t i2)
{
	void* element1 = stack_at(s, i1);
	void* element2 = stack_at(s, i2);
	memcpy(s->swap_buffer, element1, s->element_size);
	memcpy(element1, element2, s->element_size);
	memcpy(element2, s->swap_buffer, s->element_size);
}