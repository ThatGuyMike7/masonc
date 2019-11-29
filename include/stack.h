#ifndef _MASON_STACK_H_
#define _MASON_STACK_H_

#include <stdlib.h>

// Function invoked on error
extern void (*stack_cont_error_handler_proc_ptr)(char*);

// Default error handler
extern void stack_cont_error_handler_default(char* message);

// Linear list
typedef struct Stack
{
	void* buffer;
	
	// By how much the buffer grows on re-allocation in bytes
	size_t chunk_size;
	
	// Size of the buffer in bytes
	size_t size;
	
	// Size of one element in bytes
	size_t element_size;

	// How many elements the buffer contains
	size_t length;
	
} Stack;

// Allocation and initialization
extern Stack stack_new(size_t element_size, size_t chunk_element_count);

// Free all your elements first if they contain any owning pointers
extern void stack_free(Stack* s);

// Reallocate the stack to its current size plus "chunk_size" bytes
extern void stack_grow(Stack* s);

// Copy an element onto the stack
extern void stack_push(Stack* s, void* element);

// If you pop something, make sure to free() any memory first if your element has any owning pointers
extern void stack_pop(Stack* s);

// Get element at "index"
// Does not check for out of bounds
extern void* stack_at(Stack* s, size_t index);

#endif