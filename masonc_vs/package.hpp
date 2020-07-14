#ifndef $_MASON_PACKAGE_HPP_$
#define $_MASON_PACKAGE_HPP_$

#include "common.hpp"
#include "scope.hpp"

#include <string>
#include <unordered_map>
#include <set>

namespace masonc
{
	inline scope PACKAGE_SCOPE_TEMPLATE{ "" };

	struct package_import
	{
		std::string package_name;
	};
	
	struct package
	{
		std::set<package_import> imports;
		scope package_scope = PACKAGE_SCOPE_TEMPLATE;
	};
	
	// Package names associated with package structures
	using package_map = std::unordered_map<std::string, package>;
}

#endif