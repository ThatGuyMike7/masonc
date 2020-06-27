#ifndef $_MASON_PACKAGE_HPP_$
#define $_MASON_PACKAGE_HPP_$

#include <string>
#include <vector>

namespace masonc
{
	// Name of the global module
	const std::string GLOBAL_PACKAGE_NAME = "$GLOBP$";

	struct package_import
	{
		std::string package_name;
	};

	struct package
	{
		std::string package_name;
		std::vector<package_import> imports;
	};
}

#endif