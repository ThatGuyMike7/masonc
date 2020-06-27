#ifndef $_MASONC_LANG_HPP_$
#define $_MASONC_LANG_HPP_$

#include "type.hpp"
#include "binary_operator.hpp"

namespace masonc
{
	// Insert language-defined types into the global scope
	void initialize_language();
	
	// Get the operator structure if it is a defined operator
	result<const binary_operator*> get_op(s8 op_code);

	// Types defined and built into the programming language
	inline const type TYPE_VOID{ "void" };

	inline const type TYPE_BOOL{ "bool" };
	inline const type TYPE_CHAR{ "char" };
	inline const type TYPE_U8{ "u8" };
	inline const type TYPE_S8{ "s8" };

	inline const type TYPE_U16{ "u16" };
	inline const type TYPE_S16{ "s16" };

	inline const type TYPE_U32{ "u32" };
	inline const type TYPE_S32{ "s32" };
	inline const type TYPE_F32{ "f32" };
	
	inline const type TYPE_U64{ "u64" };
	inline const type TYPE_S64{ "s64" };
	inline const type TYPE_F64{ "f64" };

	inline const binary_operator OP_EQUALS{ 1, '=' };
	inline const binary_operator OP_ADD{ 2, '+' };
	inline const binary_operator OP_SUB{ 2, '-' };
	inline const binary_operator OP_MUL{ 3, '*' };
	inline const binary_operator OP_DIV{ 3, '/' };
	
}

#endif