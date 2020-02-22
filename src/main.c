#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#include <common.h>
#include <lang.h>
#include <stack.h>
#include <file_io.h>
#include <lexer.h>
#include <parser.h>

// Initial size of the input string buffer
const size_t INPUT_BUFFER_SIZE = 128;
char* get_input();

clock_t clock_get_time()
{
	clock_t _clock_t = clock();
	if(_clock_t == -1)
		quit("Error getting clock time");
	return _clock_t;
}

int main(int argc, char* argv[])
{
	init_binary_operators();
	
	File test_file = file_read("test.mason");

	clock_t clock_start;
	clock_t clock_end;
	double time_lexer, time_parser;


	clock_start = clock_get_time();
	TokenStack token_stack = lexer_eval_tokens(&test_file);
	clock_end = clock_get_time();

	time_lexer = (long double)(clock_end - clock_start) / CLOCKS_PER_SEC;
	printf("Lexer: %zu tokens in %fs\n", token_stack.tokens.length, time_lexer);


	clock_start = clock_get_time();
	Stack expr_stack = parse(&token_stack);
	clock_end = clock_get_time();

	time_parser = (long double)(clock_end - clock_start) / CLOCKS_PER_SEC;
	printf("Parser: %zu expressions in %fs\n", expr_stack.length, time_parser);

	printf("Frontend time: %f\n\n", time_lexer + time_parser);
	
	
	// Print the parsed expressions
	for(size_t i = 0; i < expr_stack.length; i += 1)
	{
		ASTExpr* expr = stack_at(&expr_stack, i);
		switch(expr->type)
		{
			case EXPR_EOF:
				printf("Expression: EOF\n");
				break;
			case EXPR_VAR:
				printf("Expression: Variable Reference\n    Literal: ");
				printf(expr->var_expr.literal);
				printf("\n");
				break;
			case EXPR_VAR_DECL:
				printf("Expression: Variable Declaration\n    Literal: ");
				printf(expr->var_decl_expr.literal);
				printf("\n    Type: ");
				
				switch(expr->var_decl_expr.lang_type)
				{
					case TYPE_F64:
						printf("f64");
						break;
					default:
						printf("Unknown");
						break;
				}

				printf("\n");
				break;
			case EXPR_NUMBER:
				printf("Expression: Number\n    Literal: ");
				printf("%ld", expr->number_expr.literal);
				printf("\n");
				break;
			case EXPR_CALL:
				printf("Expression: Call\n    Literal: ");
				printf(expr->call_expr.literal);
				printf("\n");
				break;
			case EXPR_BINARY_OPERATOR:
				printf("Expression: Binary Operator\n    Op Code: %c\n", expr->binary_operator_expr.op_code);
				break;
			default:
				printf("Expression: Unknown\n    ");
				break;
		}
	}
	
	stack_free(&expr_stack);
	lexer_free_tokens(&token_stack);
	free(test_file.buffer);
	
	free_binary_operators();
	
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