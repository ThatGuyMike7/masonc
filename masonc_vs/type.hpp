#ifndef $_MASONC_TYPE_HPP_$
#define $_MASONC_TYPE_HPP_$

#include <string>

namespace masonc
{
	struct type
	{
		std::string name;
		
		// Empty type, members uninitialized
		type() { }

		type(const std::string& name)
			: name(name)
		{ }
		
		bool operator< (const type& rhs) const
		{
			return name < rhs.name;
		}
	};
}

#endif