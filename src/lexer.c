#include <lexer.h>

#include <common.h>
#include <string_builder.h>
#include <stack.h>

#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

const size_t IDENTIFIER_STACK_CHUNK_ELEMENTS = 1024;
const size_t NUMBER_STACK_CHUNK_ELEMENTS = 1024;
const size_t TOKEN_STACK_CHUNK_ELEMENTS = 1024;

const char* ID_PROC = "proc";
const char* ID_EXTERN = "extern";

Stack identifier_stack;
Stack number_stack;

void lexer_init()
{
	identifier_stack = stack_new(sizeof(char*), IDENTIFIER_STACK_CHUNK_ELEMENTS);
	number_stack = stack_new(sizeof(double), NUMBER_STACK_CHUNK_ELEMENTS);
}

void lexer_free()
{
	for(size_t i = 0; i < identifier_stack.length; i++)
	{
		char* element = *((char**)stack_at(&identifier_stack, i));
		free(element);
	}
	stack_free(&identifier_stack);
	stack_free(&number_stack);
}

extern Token create_token(int token_type)
{
	Token token;
	token.token_type = token_type;
	return token;
}

extern Token create_tokeni(int token_type, size_t token_index)
{
	Token token;
	token.token_type = token_type;
	token.token_index = token_index;
	return token;
}

Stack lexer_get_tokens(File* file)
{
	LinearFileScanner lfs;
	lfs.file = file;
	lfs.index = 0;
	
	Stack token_stack = stack_new(sizeof(Token), TOKEN_STACK_CHUNK_ELEMENTS);
	int eof_reached = 0;
	
	while(eof_reached == 0)
	{
		char last_char = file_lfs_getchar(&lfs);
		
		// Skip whitespace
		while(isspace(last_char))
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
			if(strcmp(identifier_sb.buffer, ID_PROC) == 0)
			{
				Token token = create_token(TOKEN_PROC);
				stack_push(&token_stack, &token);
				is_variable_token = 0;
			}
			else if(strcmp(identifier_sb.buffer, ID_EXTERN) == 0)
			{
				Token token = create_token(TOKEN_EXTERN);
				stack_push(&token_stack, &token);
				is_variable_token = 0;
			}
			
			// Language defined identifier
			if(is_variable_token == 0)
			{
				sb_free(&identifier_sb);
			}
			// Variable token
			else
			{
				// Add the token and variable value
				Token token = create_tokeni(TOKEN_IDENTIFIER, identifier_stack.length);
				stack_push(&token_stack, &token);
				stack_push(&identifier_stack, identifier_sb.buffer);
				//sb_free(&identifier_sb);
			}
			
			// Note that last_char is now the character after the lexed identifier and must be processed //
			
			// Skip whitespace
			if(isspace(last_char))
				continue;
		}
		
		// Number
		// Note that this isn’t doing sufficient error checking: It will incorrectly read “1.23.45.67” and handle it as if you typed in “1.23”
		if(isdigit(last_char) || last_char == '.')
		{
			StringBuilder num_sb = sb_new(64);
			
			do
			{
				sb_push_char(&num_sb, last_char);
				last_char = file_lfs_getchar(&lfs);
			}
			while (isdigit(last_char) || last_char == '.');
			
			// Convert the string to double
			double num_value = strtod(num_sb.buffer, 0);
			sb_free(&num_sb);
			
			// Add the token and variable value
			Token token = create_tokeni(TOKEN_NUMBER, number_stack.length);
			stack_push(&token_stack, &token);
			stack_push(&number_stack, &num_value);
			
			continue;
		}
		
		// Comment until end of line
		if(last_char == '#')
		{
			do
			{
				last_char = file_lfs_getchar(&lfs);
			}
			while (last_char != '\0' && last_char != '\n' && last_char != '\t' && last_char != '\r');
			
			if(last_char != '\0')
			{
				continue;
			}
		}
		
		// Check for end of file
		if(last_char == EOF)
		{
			Token token = create_token(TOKEN_EOF);
			stack_push(&token_stack, &token);
			eof_reached = 1;
			break;
		}
			
		// Otherwise, just return the character as its ASCII value
		int ascii_token = last_char;
		Token token = create_token(ascii_token);
		stack_push(&token_stack, &token);
	}
	
	// Return all tokens
	return token_stack;
}