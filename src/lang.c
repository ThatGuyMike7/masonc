#include <lang.h>

// ----- Language-defined identifiers -----
const char* ID_PROC = "proc";
const char* ID_EXTERN = "extern";
const char* ID_STRUCT = "struct";
// ----------------------------------------

// ----- Language-defined operators -----
/*
const BinaryOperator OP_ADD = { '+', 1 };
const BinaryOperator OP_SUB = { '-', 1 };
const BinaryOperator OP_MUL = { '*', 2 };
const BinaryOperator OP_DIV = { '/', 2 };
*/

typedef struct BinaryOperator
{
	int op_code;
	int precedence;

} BinaryOperator;

SMap binary_operators;

//const BinaryOperator OP_EQUALS = { '=', 4 };
//const BinaryOperator OP_LESS = { '<', 3 };
//const BinaryOperator OP_GREATER = { '>', 3 };

int binary_operator_smap_comparator(Stack* keys, void* key)
{
	int typed_key = *((int*)key);
	for(size_t i = 0; i < keys->length; i += 1)
	{
		void* to_compare = stack_at(keys, i);
		int typed_to_compare = *((int*)to_compare);
		
		if(typed_key == typed_to_compare)
			return (int)i;
	}
	return -1;
}

void init_binary_operators()
{
	BinaryOperator OP_ADD = { '+', 1 };
	BinaryOperator OP_SUB = { '-', 1 };
	BinaryOperator OP_MUL = { '*', 2 };
	BinaryOperator OP_DIV = { '/', 2 };
	
	binary_operators = smap_new(sizeof(int), sizeof(int), 128, &binary_operator_smap_comparator);
	smap_push(&binary_operators, &OP_ADD.op_code, &OP_ADD.precedence);
	smap_push(&binary_operators, &OP_SUB.op_code, &OP_SUB.precedence);
	smap_push(&binary_operators, &OP_MUL.op_code, &OP_MUL.precedence);
	smap_push(&binary_operators, &OP_DIV.op_code, &OP_DIV.precedence);
}

void free_binary_operators()
{
	smap_free(&binary_operators);
}

/*
int get_binary_op_precedence(char op_code)
{
	switch(op_code)
	{
		case '+':
			return OP_ADD.precedence;
			break;
		
		case '-':
			return OP_SUB.precedence;
			break;
		
		case '*':
			return OP_MUL.precedence;
			break;
		
		case '/':
			return OP_DIV.precedence;
			break;
		
		default:
			return -1;
			break;
	}
}
*/
// --------------------------------------