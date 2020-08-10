#ifndef $_MASONC_BINARY_OPERATOR_HPP_$
#define $_MASONC_BINARY_OPERATOR_HPP_$

#include "common.hpp"

namespace masonc
{
	struct binary_operator
	{
		u32 precedence;
		s8 op_code;
	};
}

#endif