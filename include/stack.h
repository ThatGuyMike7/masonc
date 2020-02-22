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
	
	// Buffer with size of 1 element for the swap operation
	void* swap_buffer;
	
} Stack;

// Allocation and initialization
extern Stack stack_new(size_t element_size, size_t chunk_element_count);

// Free all your elements first if they contain any owning pointers
extern void stack_free(Stack* s);

// Reallocate the stack to its current size plus 'chunk_size * chunks' bytes
extern void stack_grow(Stack* s, size_t chunks);

// Copy an element onto the stack
extern void stack_push(Stack* s, void* element);

// Copy the source stack's buffer and add it onto the destination stack.
// 
// The destination's 'element_size' and 'chunk_size' are used. This results in the type having to be equal in both stacks
// (element_size, more specifically. But it is recommended to have equal types unless you know what you are doing).
extern void stack_pushs(Stack* dest_stack, Stack* src_stack);

// If you pop something, make sure to free() any memory first if your element has any owning pointers
extern void stack_pop(Stack* s);

// Get element at "index"
// Does not check for out of bounds
extern void* stack_at(Stack* s, size_t index);

// Swap two elements at indices i1 and i2
extern void stack_swap(Stack* s, size_t i1, size_t i2);

#endif