#include "lang.hpp"
#include "scope.hpp"
#include "package.hpp"

#include <optional>

namespace masonc
{
	// Insert language-defined types into `PACKAGE_SCOPE_TEMPLATE`
	// TODO: Turn `PACKAGE_SCOPE_TEMPLATE` const and make this function constexpr
	void initialize_language()
	{
		//scope_template.types.insert(std::begin(LANGUAGE_DEFINED_TYPES), std::end(LANGUAGE_DEFINED_TYPES));
		PACKAGE_SCOPE_TEMPLATE.add_type(type{ TYPE_VOID }, PACKAGE_SCOPE_TEMPLATE);

		PACKAGE_SCOPE_TEMPLATE.add_type(type{ TYPE_BOOL }, PACKAGE_SCOPE_TEMPLATE);
		PACKAGE_SCOPE_TEMPLATE.add_type(type{ TYPE_CHAR }, PACKAGE_SCOPE_TEMPLATE);
		PACKAGE_SCOPE_TEMPLATE.add_type(type{ TYPE_U8 }, PACKAGE_SCOPE_TEMPLATE);
		PACKAGE_SCOPE_TEMPLATE.add_type(type{ TYPE_S8 }, PACKAGE_SCOPE_TEMPLATE);

		PACKAGE_SCOPE_TEMPLATE.add_type(type{ TYPE_U16 }, PACKAGE_SCOPE_TEMPLATE);
		PACKAGE_SCOPE_TEMPLATE.add_type(type{ TYPE_S16 }, PACKAGE_SCOPE_TEMPLATE);

		PACKAGE_SCOPE_TEMPLATE.add_type(type{ TYPE_U32 }, PACKAGE_SCOPE_TEMPLATE);
		PACKAGE_SCOPE_TEMPLATE.add_type(type{ TYPE_S32 }, PACKAGE_SCOPE_TEMPLATE);
		PACKAGE_SCOPE_TEMPLATE.add_type(type{ TYPE_F32 }, PACKAGE_SCOPE_TEMPLATE);

		PACKAGE_SCOPE_TEMPLATE.add_type(type{ TYPE_U64 }, PACKAGE_SCOPE_TEMPLATE);
		PACKAGE_SCOPE_TEMPLATE.add_type(type{ TYPE_S64 }, PACKAGE_SCOPE_TEMPLATE);
		PACKAGE_SCOPE_TEMPLATE.add_type(type{ TYPE_F64 }, PACKAGE_SCOPE_TEMPLATE);
	}

	std::optional<const binary_operator*> get_op(s8 op_code)
	{
		if (op_code == OP_EQUALS.op_code)
			return std::optional<const binary_operator*>{ &OP_EQUALS };
		if (op_code == OP_ADD.op_code)
			return std::optional<const binary_operator*>{ &OP_ADD };
		if (op_code == OP_SUB.op_code)
			return std::optional<const binary_operator*>{ &OP_SUB };
		if (op_code == OP_MUL.op_code)
			return std::optional<const binary_operator*>{ &OP_MUL };
		if (op_code == OP_DIV.op_code)
			return std::optional<const binary_operator*>{ &OP_DIV };

		return std::optional<const binary_operator*>{};
	}
}
