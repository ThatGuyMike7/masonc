#ifndef _MASON_LANG_H_
#define _MASON_LANG_H_

#include <smap.h>

// ----- Language-defined identifiers -----
extern const char* ID_PROC;
extern const char* ID_EXTERN;
extern const char* ID_STRUCT;
// ----------------------------------------

// ----- Language-defined types -----
enum LanguageType
{
	TYPE_F64
};
// ----------------------------------

// ----- Language-defined operators -----

// Map of (operator,precedence) pairs of type (int,int)
extern SMap binary_operators;

/*
extern const BinaryOperator OP_ADD;
extern const BinaryOperator OP_SUB;
extern const BinaryOperator OP_MUL;
extern const BinaryOperator OP_DIV;
*/

//extern const BinaryOperator OP_EQUALS;
//extern const BinaryOperator OP_LESS;
//extern const BinaryOperator OP_GREATER;

extern void init_binary_operators();
extern void free_binary_operators();

// -1 if given binary operator is undefined
//extern int get_binary_op_precedence(char op_code); 
// --------------------------------------


#endif