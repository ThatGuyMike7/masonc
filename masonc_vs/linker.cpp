#include "linker.hpp"

namespace masonc
{
	void pre_linker::merge_global_scopes(scope& destination_global, scope& source_global)
	{
		// Copy symbols from `source_global` to `destination_global`
		for (auto it = source_global.symbols.begin(); it != source_global.symbols.end(); it++)
		{
			const std::string& key = it->first;

			auto find_key_it = destination_global.symbols.find(key);
			if (find_key_it != destination_global.symbols.end())
			{
				// TODO: Finish this
				// Generate an error because a symbol with the same name already exists in `source_global`
				output->messages.report_error("");
			}
		}
	}
}