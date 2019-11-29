#ifndef _LEXER_H_
#define _LEXER_H_

#include <file_io.h>
#include <stack.h>

// Language-defined identifiers
extern const char* ID_PROC;
extern const char* ID_EXTERN;

// Variable value tokens (filled for TOKEN_IDENTIFIER, TOKEN_NUMBER, ...)
extern Stack identifier_stack;
extern Stack number_stack;

// The lexer returns tokens [0-255] if it is an unknown character, otherwise one of these things
enum TokenType
{
	TOKEN_EOF = -1,
	
	// Tokens with variable values
	TOKEN_IDENTIFIER = -2,
	TOKEN_NUMBER = -3,
	
	// Language-defined identifiers
	TOKEN_PROC = -4,
	TOKEN_EXTERN = -5,
};

typedef struct Token
{
	// TokenType
	int token_type;
	
	// Optional index to read the token's value (TOKEN_IDENTIFIER, TOKEN_NUMBER, ...)
	size_t token_index;
} Token;

extern void lexer_init();
extern void lexer_free();

extern Token create_token(int token_type);
extern Token create_tokeni(int token_type, size_t token_index);

// Return all tokens from a source file. The variable tokens are added to global stacks.
extern Stack lexer_get_tokens(File* file);

#endif