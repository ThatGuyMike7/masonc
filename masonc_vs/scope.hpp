#ifndef $_MASONC_SCOPE_HPP_$
#define $_MASONC_SCOPE_HPP_$

#include "common.hpp"
#include "type.hpp"
#include "symbol.hpp"
#include <string>
#include <vector>
#include <unordered_map>

namespace masonc
{
	struct scope_index
	{
		std::vector<u64> indices;
	};

	struct scope
	{
		// Scope identifier.
		//
		// Unique scopes where declaration order matters such as procedures must have
		// an identifier starting with "$".
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

		// Add a child scope, give it the correct Scope_Index and return it
		scope_index add_child(const scope& child);
		scope* get_child(const scope_index& si);

		// Returns false if the symbol is already defined in this scope or a parent scope
		bool add_symbol(const symbol& s, scope& global_scope);

		// Returns false if the type is already defined in this scope or a parent scope
		bool add_type(const type& t, scope& global_scope);

		// Search for a type from top to bottom starting at the global scope until this scope
		result<type> find_type(const std::string& type_name, scope& global_scope);

		// Search for a symbol from top to bottom starting at the global scope until this scope
		result<symbol> find_symbol(const std::string& symbol_name, scope& global_scope);
	};

	inline scope GLOBAL_SCOPE_TEMPLATE{ "global" };
}

#endif