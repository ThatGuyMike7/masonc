#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <common.h>
#include <stack.h>
#include <file_io.h>
#include <lexer.h>

// Initial size of the input string buffer
const size_t INPUT_BUFFER_SIZE = 128;
char* get_input();

int main(int argc, char* argv[])
{
	lexer_init();
	
	File test_file = file_read("test.mason");
	Stack token_stack = lexer_get_tokens(&test_file);
	
	for(size_t i = 0; i < token_stack.length; i++)
	{
		Token* token = ((Token*)stack_at(&token_stack, i));
		switch(token->token_type)
		{
			case TOKEN_EXTERN:
				printf("extern keyword\n");
				break;
			case TOKEN_PROC:
				printf("proc keyword\n");
				break;
			case TOKEN_IDENTIFIER:
				printf("identifier: ");
				char* element = *((char**)stack_at(&identifier_stack, token->token_index));
				printf(element);
				printf("\n");
				break;
			case TOKEN_NUMBER:
				printf("number: ");
				//double d = *((double*)stack_at(&number_stack, token->token_index));
				//char arr[sizeof(d)];
				//memcpy(arr, &d, sizeof(d));
				//printf(arr);
				printf("\n");
				break;
			case TOKEN_EOF:
				printf("END OF FILE\n");
				break;
			default:
				printf("token: ");
				putchar(token->token_type);
				printf("\n");
				break;
		}
	}
	
	stack_free(&token_stack);
	free(test_file.buffer);
	//lexer_free();
	
	printf("\n\nProgram terminates\n");
	printf("Press return to exit\n");
	getchar();
	return 0;
}

char* get_input()
{
	size_t max = INPUT_BUFFER_SIZE;
	
	// Allocate buffer
	// "char" is guaranteed to be 1 byte by the C standard
	char* buffer = (char*)malloc(max);
	if(buffer == NULL)
		quit("Input memory exhausted");
	
	size_t i = 0;
	while(1)
	{
		int c = getchar();
		
		if(c == '\n')
		{
			buffer[i] = '\0';
			break;
		}
		
		buffer[i] = c;
		
		if(i == max - 1)
		{
			printf("Resizing buffer\n");
			max += max;
			
			// Get a new and larger buffer otherwise we run out of memory
			buffer = (char*)realloc(buffer, max);
			if(buffer == NULL)
				quit("Input memory exhausted");
		}
		
		i++;
	}
	
	return buffer;
}