#ifndef _MASON_PARSER_H_
#define _MASON_PARSER_H_

#include <stack.h>
#include <lexer.h>
#include <lang.h>

enum ASTExprType
{
	// Indicating end of file (empty expression)
	EXPR_EOF,

	EXPR_NUMBER,
	EXPR_VAR,
	EXPR_VAR_DECL,
	EXPR_PROTOTYPE,
	EXPR_PROC,
	EXPR_CALL,
	EXPR_BINARY_OPERATOR
};

// Stack of ASTExpr
// Examples:
// - statement (stack of expressions)
// - procedure argument (stack of expressions, too)
typedef Stack ASTExprStack;

// Stack of ASTExprStack
// Examples:
// - procedure body (a statement is a stack of expressions, and a body is a stack of statements),
// - arguments in function call (one argument can be composed of multiple expressions, and a procedure can expect multiple arguments)
typedef Stack ASTList;

// Numeric literals
// := "5"
typedef struct ASTNumberExpr
{
	long literal;
	
} ASTNumberExpr;

// Referencing variables
// := "x"
typedef struct ASTVarExpr
{
	char* literal;
	
} ASTVarExpr;

// Declaring variables
// := "n: s64"
typedef struct ASTVarDeclExpr
{
	int lang_type;
	char* literal;
	
} ASTVarDeclExpr;

// Expression for procedure calls
typedef struct ASTCallExpr
{
	ASTList arguments;
	char* literal;
	
} ASTCallExpr;

// Declaring procedures
// := "fibonacci :: (n: s64);"
typedef struct ASTProcPrototypeExpr
{
	// Type 'ASTVarDeclExpr'
	ASTExprStack argument_list;
	char* literal;
	
} ASTProcPrototypeExpr;

// Defining procedures (prototype + body)
typedef struct ASTProcExpr
{
	ASTProcPrototypeExpr prototype;
	ASTList body;
	
} ASTProcExpr;

typedef struct ASTBinaryOperatorExpr
{
	char op_code;
	
} ASTBinaryOperatorExpr;

typedef struct ASTExpr
{
	// Which type of expression the union represents
	int type;
	
	// The expression union
	union ASTExprUnion
	{
		ASTNumberExpr number_expr;
		ASTVarExpr var_expr;
		ASTVarDeclExpr var_decl_expr;
		ASTCallExpr call_expr;
		ASTProcPrototypeExpr proc_prototype_expr;
		ASTProcExpr proc_expr;
		ASTBinaryOperatorExpr binary_operator_expr;
	};
	
} ASTExpr;

typedef struct Parser
{
	// Tokens to be parsed into expressions
	TokenStack* token_stack;
	
	Token* current_token;
	size_t current_token_index;
	
	// Parsed expressions
	Stack expressions;
	
	int EOF_reached;
	
} Parser;

// Evaluates the given tokens and parses all expressions.
extern Stack parse(TokenStack* token_stack);

#endif