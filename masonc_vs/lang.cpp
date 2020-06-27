#include "lang.hpp"
#include "scope.hpp"
#include "common.hpp"

#include <iterator>

namespace masonc
{
	void initialize_language()
	{
		//GLOBAL_SCOPE_TEMPLATE.types.insert(std::begin(LANGUAGE_DEFINED_TYPES), std::end(LANGUAGE_DEFINED_TYPES));
		GLOBAL_SCOPE_TEMPLATE.add_type(type{ TYPE_VOID }, GLOBAL_SCOPE_TEMPLATE);
		
		GLOBAL_SCOPE_TEMPLATE.add_type(type{ TYPE_BOOL }, GLOBAL_SCOPE_TEMPLATE);
		GLOBAL_SCOPE_TEMPLATE.add_type(type{ TYPE_CHAR }, GLOBAL_SCOPE_TEMPLATE);
		GLOBAL_SCOPE_TEMPLATE.add_type(type{ TYPE_U8 }, GLOBAL_SCOPE_TEMPLATE);
		GLOBAL_SCOPE_TEMPLATE.add_type(type{ TYPE_S8 }, GLOBAL_SCOPE_TEMPLATE);

		GLOBAL_SCOPE_TEMPLATE.add_type(type{ TYPE_U16 }, GLOBAL_SCOPE_TEMPLATE);
		GLOBAL_SCOPE_TEMPLATE.add_type(type{ TYPE_S16 }, GLOBAL_SCOPE_TEMPLATE);

		GLOBAL_SCOPE_TEMPLATE.add_type(type{ TYPE_U32 }, GLOBAL_SCOPE_TEMPLATE);
		GLOBAL_SCOPE_TEMPLATE.add_type(type{ TYPE_S32 }, GLOBAL_SCOPE_TEMPLATE);
		GLOBAL_SCOPE_TEMPLATE.add_type(type{ TYPE_F32 }, GLOBAL_SCOPE_TEMPLATE);

		GLOBAL_SCOPE_TEMPLATE.add_type(type{ TYPE_U64 }, GLOBAL_SCOPE_TEMPLATE);
		GLOBAL_SCOPE_TEMPLATE.add_type(type{ TYPE_S64 }, GLOBAL_SCOPE_TEMPLATE);
		GLOBAL_SCOPE_TEMPLATE.add_type(type{ TYPE_F64 }, GLOBAL_SCOPE_TEMPLATE);
	}

	result<const binary_operator*> get_op(s8 op_code)
	{
		if (op_code == OP_EQUALS.op_code)
			return result<const binary_operator*>{ &OP_EQUALS };
		if (op_code == OP_ADD.op_code)
			return result<const binary_operator*>{ &OP_ADD };
		if (op_code == OP_SUB.op_code)
			return result<const binary_operator*>{ &OP_SUB };
		if (op_code == OP_MUL.op_code)
			return result<const binary_operator*>{ &OP_MUL };
		if (op_code == OP_DIV.op_code)
			return result<const binary_operator*>{ &OP_DIV };

		return result<const binary_operator*>{};
	}
}
