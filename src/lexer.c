#include <lexer.h>

#include <common.h>
#include <string_builder.h>
#include <stack.h>
#include <text.h>

#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

const size_t IDENTIFIER_STACK_CHUNK_ELEMENTS = 1024;
const size_t NUMBER_STACK_CHUNK_ELEMENTS = 1024;
const size_t TOKEN_STACK_CHUNK_ELEMENTS = 1024;

Token lexer_create_token(int token_type)
{
	Token token;
	token.token_type = token_type;
	return token;
}

Token lexer_create_tokeni(int token_type, size_t value_index)
{
	Token token;
	token.token_type = token_type;
	token.value_index = value_index;
	return token;
}

Token* lexer_get_token(Stack* tokens, size_t token_index)
{
	return ((Token*)stack_at(tokens, token_index));
}

char* lexer_get_identifier(Stack* identifier_stack, size_t value_index)
{
	return *((char**)stack_at(identifier_stack, value_index));
}

long lexer_get_number(Stack* number_stack, size_t value_index)
{
	return *((long*)stack_at(number_stack, value_index));
}

TokenStack lexer_eval_tokens(File* file)
{
	LinearFileScanner lfs;
	lfs.file = file;
	lfs.index = 0;
	
	TokenStack token_stack;
	token_stack.tokens = stack_new(sizeof(Token), TOKEN_STACK_CHUNK_ELEMENTS);
	token_stack.identifiers = stack_new(sizeof(char*), IDENTIFIER_STACK_CHUNK_ELEMENTS);
	token_stack.numbers = stack_new(sizeof(double), NUMBER_STACK_CHUNK_ELEMENTS);
	
	int get_next_char = 1;
	char last_char;
	
	while(1)
	{
		// Check if we need to get the next character for lexing
		if(get_next_char != 0)
		{
			last_char = file_lfs_getchar(&lfs);
			get_next_char = 0;
		}
		
		// Skip whitespace
		while(text_is_space(last_char))
			last_char = file_lfs_getchar(&lfs);
		
		// Identifier starting with an alpha character (a-z, A-Z) and further characters being alpha or numeric (0-9)
		if(isalpha(last_char))
		{
			StringBuilder identifier_sb = sb_new(64);
			int is_variable_token = 1;
			
			// Read the identifier
			do
			{
				sb_push_char(&identifier_sb, last_char);
				last_char = file_lfs_getchar(&lfs);
			}
			while (isalnum(last_char));
			
			// Check if it is a language-defined identifier
			if(strcmp(identifier_sb.buffer, ID_EXTERN) == 0)
			{
				Token token = lexer_create_token(TOKEN_EXTERN);
				stack_push(&token_stack.tokens, &token);
				is_variable_token = 0;
			}
			else if(strcmp(identifier_sb.buffer, ID_STRUCT) == 0)
			{
				Token token = lexer_create_token(TOKEN_STRUCT);
				stack_push(&token_stack.tokens, &token);
				is_variable_token = 0;
			}
			
			// Variable identifier
			if(is_variable_token != 0)
			{
				// Add the token and variable value
				Token token = lexer_create_tokeni(TOKEN_IDENTIFIER, token_stack.identifiers.length);
				stack_push(&token_stack.tokens, &token);
				stack_push(&token_stack.identifiers, &identifier_sb.buffer);
			}
			// Language defined identifier
			else
			{
				sb_free(&identifier_sb);
			}
			
			// last_char is now the character after the lexed identifier and must be processed
			continue;
		}
		
		// Number
		// Note that this isn’t doing sufficient error checking: It will incorrectly read “1.23.45.67” and handle it as if you typed in “1.23”
		if(isdigit(last_char)) //|| last_char == '.')
		{
			StringBuilder num_sb = sb_new(64);

			do
			{
				sb_push_char(&num_sb, last_char);
				last_char = file_lfs_getchar(&lfs);
			}
			while (isdigit(last_char)); //|| last_char == '.');
			
			// Convert the string to long
			long num_value = strtol(num_sb.buffer, 0, 0);
			sb_free(&num_sb);
			
			// Add the token and variable value
			Token token = lexer_create_tokeni(TOKEN_NUMBER, token_stack.numbers.length);
			stack_push(&token_stack.tokens, &token);
			stack_push(&token_stack.numbers, &num_value);
			
			// last_char is now the character after the lexed number and must be processed
			continue;
		}
		
		// Comment until end of line
		if(last_char == '#')
		{
			do
			{
				last_char = file_lfs_getchar(&lfs);
			}
			while (last_char != EOF && last_char != '\n' && last_char != '\r');
			
			if(last_char != EOF)
			{
				get_next_char = 1;
				continue;
			}
		}
		
		// Check for end of file
		if(last_char == EOF)
		{
			Token token = lexer_create_token(TOKEN_EOF);
			stack_push(&token_stack.tokens, &token);
			break;
		}
		
		// Composed tokens
		if(last_char == ':')
		{
			char next_char = file_lfs_getchar(&lfs);
			if(next_char == ':')
			{
				Token token = lexer_create_token(TOKEN_DOUBLECOLON);
				stack_push(&token_stack.tokens, &token);
				get_next_char = 1;
				continue;
			}
		}
		
		// Otherwise just add the character as its ASCII value
		int ascii_token = last_char;
		Token token = lexer_create_token(ascii_token);
		stack_push(&token_stack.tokens, &token);
		
		// Get the next character in the next iteration
		get_next_char = 1;
		continue;
	}

	// Return all tokens
	return token_stack;
}

void lexer_free_tokens(TokenStack* token_stack)
{
	for(size_t i = 0; i < token_stack->identifiers.length; i++)
	{
		char* element = lexer_get_identifier(&token_stack->identifiers, i);
		free(element);
	}
	stack_free(&token_stack->identifiers);
	stack_free(&token_stack->numbers);
}