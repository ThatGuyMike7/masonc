#ifndef $_MASONC_SYMBOL_HPP_$
#define $_MASONC_SYMBOL_HPP_$

#include "type.hpp"
#include "common.hpp"
#include <string>

namespace masonc
{	
	enum symbol_type : s16
	{
		SYMBOL_UNSET,
		SYMBOL_VARIABLE,
		SYMBOL_PROCEDURE
	};

	struct symbol
	{
		std::string name;
		symbol_type type = SYMBOL_UNSET;

		// Optional type name that is set when symbol is a variable
		std::string variable_type_name;
		
		bool operator< (const symbol& rhs) const
		{
			return name < rhs.name;
		}
	};
}

#endif