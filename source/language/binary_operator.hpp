#ifndef MASONC_BINARY_OPERATOR_HPP
#define MASONC_BINARY_OPERATOR_HPP

#include <common.hpp>

#include <optional>

namespace masonc
{
    struct binary_operator
	{
		u32 precedence;
		s8 op_code;
	};

    // Get the operator structure if it is a defined operator.
    std::optional<const binary_operator*> get_op(s8 op_code);

    inline const binary_operator OP_EQUALS{ 1, '=' };
    inline const binary_operator OP_ADD{ 2, '+' };
    inline const binary_operator OP_SUB{ 2, '-' };
    inline const binary_operator OP_MUL{ 3, '*' };
    inline const binary_operator OP_DIV{ 3, '/' };
}

#endif