#ifndef _MASON_LEXER_H_
#define _MASON_LEXER_H_

#include <file_io.h>
#include <stack.h>
#include <lang.h>

// The lexer returns tokens [0-255] if it is an unknown character, otherwise one of these things
enum TokenType
{
	TOKEN_EOF = -1,
	
	// ----- Tokens with variable values -----
	TOKEN_IDENTIFIER = -2,
	TOKEN_NUMBER = -3,
	// ---------------------------------------
	
	// ----------- Composed tokens -----------
	// ::
	TOKEN_DOUBLECOLON = -4,
	// ---------------------------------------
	
	// ----- Language-defined identifiers -----
	TOKEN_EXTERN = -5,
	TOKEN_STRUCT = -6
	// ----------------------------------------
};

typedef struct Token
{
	// TokenType
	int token_type;
	
	// Optional index to read the token's value (TOKEN_IDENTIFIER, TOKEN_NUMBER, ...)
	size_t value_index;
	
} Token;

typedef struct TokenStack
{
	Stack tokens;
	
	// Variable value tokens (filled for TOKEN_IDENTIFIER, TOKEN_NUMBER, ...)
	Stack identifiers;
	Stack numbers;
	
} TokenStack;

// -------------------- HELPER FUNCTIONS --------------------  
// -----    For avoiding pointer casting mistakes.      -----
// -----    Lexer must have evaluated tokens first.     -----
// ----------------------------------------------------------
// Get a token at a specific index
extern Token* lexer_get_token(Stack* tokens, size_t token_index);

// Get an identifier at a specific index
extern char* lexer_get_identifier(Stack* identifier_stack, size_t value_index);

// Get a number at a specific index
extern long lexer_get_number(Stack* number_stack, size_t value_index);
// ----------------------------------------------------------
// ----------------------------------------------------------

// Return all tokens (including number and identifier values) from a source file
extern TokenStack lexer_eval_tokens(File* file);

// Free the identifier, number and token stacks
extern void lexer_free_tokens(TokenStack* token_stack);

#endif