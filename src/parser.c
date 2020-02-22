#include <parser.h>

#include <common.h>
#include <smap.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

const size_t EXPRESSION_CHUNK_ELEMENT_COUNT = 256;
const size_t STATEMENT_CHUNK_ELEMENT_COUNT = 8;

void parse_expression(Parser* parser);

void get_next_token(Parser* parser)
{
	parser->current_token_index += 1;
	parser->current_token = lexer_get_token(&parser->token_stack->tokens, parser->current_token_index);
	if(parser->current_token->token_type == TOKEN_EOF)
		parser->EOF_reached = 1;
}

Token* peek_token(Parser* parser)
{
	// Do not peek out of bounds
	if(parser->EOF_reached != 0)
		return parser->current_token;
	
	return lexer_get_token(&parser->token_stack->tokens, parser->current_token_index + 1);
}

// Numeric literal (1 token)
void parse_number_expr(Parser* parser)
{
	ASTExpr result;
	result.type = EXPR_NUMBER;
	result.number_expr.literal = lexer_get_number(&parser->token_stack->numbers, parser->current_token->value_index);
	stack_push(&parser->expressions, &result);
	
	// Eat the number
	get_next_token(parser);
}

// Literal (1 token)
void parse_var(Parser* parser, char* identifier)
{
	ASTExpr result;
	result.type = EXPR_VAR;
	result.var_expr.literal = identifier;
	stack_push(&parser->expressions, &result);
	
	// Eat the identifier
	get_next_token(parser);
}

// Literal + type
void parse_var_decl(Parser* parser, char* identifier)
{
	// Eat the identifier and ':'
	get_next_token(parser);
	get_next_token(parser);
	if(parser->EOF_reached != 0)
		quit("Expected type");
	
	if(parser->current_token->token_type != TOKEN_IDENTIFIER)
	{
		quit("Unexpected token when parsing variable declaration");
	}
	
	ASTExpr result;
	result.type = EXPR_VAR_DECL;
	result.var_decl_expr.literal = identifier;
	
	// Parse the type
	char* type_string =  lexer_get_identifier(&parser->token_stack->identifiers, parser->current_token->value_index);
	if(strcmp(type_string, "f64") == 0)
	{
		result.var_decl_expr.lang_type = TYPE_F64;
	}
	else
	{
		quit("Unknown type");
	}
	
	// Eat the type
	get_next_token(parser);

	stack_push(&parser->expressions, &result);
}

void parse_proc_call(Parser* parser, char* identifier)
{
	// Eat the identifier and '('
	get_next_token(parser);
	get_next_token(parser);
	if(parser->EOF_reached != 0)
		quit("Expected arguments");
	
	// TODO: Parse arguments and do error handling
	
	if(parser->current_token->token_type != ')')
		quit("Expected ')'");
	
	// Eat the ')'
	get_next_token(parser);

	ASTExpr result;
	result.type = EXPR_CALL;
	result.call_expr.literal = identifier;
	stack_push(&parser->expressions, &result);
}

// Parse either a prototype or a complete procedure definition.
// token_index points to '('
void parse_proc(Parser* parser, char* identifier)
{
	
	
	// TODO: Implement this
}

// '(' expression ')'
void parse_parantheses_expr(Parser* parser)
{
	// Eat '('
	get_next_token(parser);
	if(parser->EOF_reached != 0)
		return;
	
	parse_expression(parser);
	
	if(parser->current_token->token_type != ')')
		quit("Expected ')'");
	
	// Eat ')'
	get_next_token(parser);
	//if(parser->EOF_reached != 0)
	//	return;
}

// Variable reference, variable declaration, procedure call, procedure prototype, procedure definition, structs etc.
void parse_identifier_expr(Parser* parser)
{
	char* identifier = lexer_get_identifier(&parser->token_stack->identifiers, parser->current_token->value_index);
	
	Token* next_token = peek_token(parser);
	switch(next_token->token_type)
	{
		// Variable declaration
		case ':':
			parse_var_decl(parser, identifier);
			break;
					
		// Procedure call
		case '(':
			parse_proc_call(parser, identifier);
			break;
			
		// Procedure prototype, procedure definition or struct
		case TOKEN_DOUBLECOLON:
			
			break;
		
		// Variable reference
		default:
			parse_var(parser, identifier);
			break;
	}
}

// Identifier, number, parentheses
void parse_primary_expr(Parser* parser)
{
	switch(parser->current_token->token_type)
	{
		// Parse an identifier (variable reference, variable declaration, procedure call, procedure prototype, procedure definition, structs, etc.)
		case TOKEN_IDENTIFIER:
			parse_identifier_expr(parser);
			break;
			
		// Parse a numeric literal
		case TOKEN_NUMBER:
			parse_number_expr(parser);
			break;
			
		case '(':
			parse_parantheses_expr(parser);
			break;

		default:
			char msg[128];
			strcpy(msg, "Unknown token when expecting an expression: ");
			
			char token[2];
			token[0] = parser->current_token->token_type;
			token[1] = '\0';
			strcat(msg, token);
			
			quit(msg);
			break;
	}
}

// Expression is a primary expression potentially followed by a sequence of [binary_operator, primary_expression] pairs
void parse_expression(Parser* parser)
{
	parse_primary_expr(parser);

	// If next token is a binary operator, add its expression and eat the token
	void* precedence = smap_get(&binary_operators, &parser->current_token->token_type);
	if(precedence != 0)
	{
		ASTExpr result;
		result.type = EXPR_BINARY_OPERATOR;
		result.binary_operator_expr.op_code = parser->current_token->token_type;
		stack_push(&parser->expressions, &result);
		
		get_next_token(parser);
	}
}

Stack parse(TokenStack* token_stack)
{
	Parser parser;
	parser.token_stack = token_stack;
	parser.current_token_index = -1;
	parser.expressions = stack_new(sizeof(ASTExpr), EXPRESSION_CHUNK_ELEMENT_COUNT);
	parser.EOF_reached = 0;
	
	// Get the first token
	get_next_token(&parser);
	
	while(parser.EOF_reached == 0)
	{
		parse_expression(&parser);
	}
	
	// Add the EOF
	ASTExpr EOF_expr;
	EOF_expr.type = EXPR_EOF;
	stack_push(&parser.expressions, &EOF_expr);
	
	return parser.expressions;
}