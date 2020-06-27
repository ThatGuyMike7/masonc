#ifndef $_MASONC_SYMBOL_HPP_$
#define $_MASONC_SYMBOL_HPP_$

#include "type.hpp"
#include "common.hpp"
#include <string>

namespace masonc
{	
	enum mason_symbol_type : s16
	{
		SYMBOL_UNSET,
		SYMBOL_VARIABLE,
		SYMBOL_PROCEDURE
	};

	struct symbol
	{
		std::string name;
		std::string package_name;
		mason_symbol_type type = SYMBOL_UNSET;

		// Optional type name that is set when symbol is a variable
		std::string variable_type_name;
		
		std::string full_name()
		{
			return package_name + "." + name;
		}
		
		bool operator< (const symbol& rhs) const
		{
			return name < rhs.name;
		}
	};
}

#endif