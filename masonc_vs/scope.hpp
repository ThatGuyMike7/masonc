#ifndef $_MASONC_SCOPE_HPP_$
#define $_MASONC_SCOPE_HPP_$

#include "common.hpp"
#include "type.hpp"
#include "symbol.hpp"

#include <string>
#include <vector>
#include <unordered_map>
#include <optional>

namespace masonc
{
	struct scope_index
	{
		std::vector<u64> indices;
	};

	struct scope
	{
		// The scope name "*___*" is reserved and means "uninitialized".
		std::string name;

		scope_index index;

		std::unordered_map<std::string, symbol> symbols;
		std::unordered_map<std::string, type> types;
		
		std::vector<scope> children;
		
		scope()
			: name("*___*")
		{ }

		scope(const std::string& name)
			: name(name)
		{ }

		// Add a child scope, give it the correct scope_index and return the scope_index
		scope_index add_child(const scope& child);
		scope* get_child(const scope_index& si);

		// Returns false if the symbol is already defined in this scope or a parent scope
		bool add_symbol(const symbol& s, scope& package_scope);

		// Returns false if the type is already defined in this scope or a parent scope
		bool add_type(const type& t, scope& package_scope);

		// Search for a type from top to bottom starting at the package scope until this scope
		std::optional<type> find_type(const std::string& type_name, scope& package_scope);

		// Search for a symbol from top to bottom starting at the package scope until this scope
		std::optional<symbol> find_symbol(const std::string& symbol_name, scope& package_scope);
	};
}

#endif