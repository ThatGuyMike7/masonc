#include "linker.hpp"

namespace masonc
{
	void pre_linker::merge_package_scopes(scope& destination, scope& source)
	{
		// Copy symbols from source to destination
		for (auto it = source.symbols.begin(); it != source.symbols.end(); it++)
		{
			const std::string& key = it->first;

			auto find_key_it = destination.symbols.find(key);
			if (find_key_it != destination.symbols.end())
			{
				// TODO: Finish this
				
				// Generate an error because a symbol with the same name already exists in `source`
				output->messages.report_error("");
			}
		}
	}
}