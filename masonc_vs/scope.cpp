#include "scope.hpp"
#include "log.hpp"
#include "common.hpp"

#include <iostream>

namespace masonc
{
	scope_index scope::add_child(const scope& child)
	{
		assume(this->name != "*___*", "Scope name was \"*___*\"");

		u64 added_child_index = this->children.size();
		this->children.push_back(child);

		scope& added_child = this->children[added_child_index];
		added_child.index = this->index;
		added_child.index.indices.push_back(added_child_index);

		return added_child.index;
	}

	scope* scope::get_child(const scope_index& si)
	{
		assume(this->name != "*___*", "Scope name was \"*___*\"");

		scope* s = this;

		for (u64 i = s->index.indices.size(); i < si.indices.size(); i += 1)
		{
			s = &s->children[si.indices[i]];
		}

		return s;
	}

	bool scope::add_symbol(const symbol& s, scope& global_scope)
	{
		assume(this->name != "*___*", "Scope name was \"*___*\"");

		result<symbol> search_result = find_symbol(s.name, global_scope);
		if (search_result)
			return false;
		
		this->symbols.insert({ s.name, s });
		return true;
	}

	bool scope::add_type(const type& t, scope& global_scope)
	{
		assume(this->name != "*___*", "Scope name was \"*___*\"");

		result<type> search_result = find_type(t.name, global_scope);
		if (search_result)
			return false;

		this->types.insert({ t.name, t });
		return true;
	}

	result<type> scope::find_type(const std::string& type_name, scope& global_scope)
	{
		assume(this->name != "*___*", "Scope name was \"*___*\"");

		scope_index current_scope_index = global_scope.index;
		u64 i = 0;

		while (true)
		{
			scope* child = global_scope.get_child(current_scope_index);

			auto it = child->types.find(type_name);
			if (it != child->types.end())
			{
				return result<type>{ it->second };
			}

			if (i == this->index.indices.size())
				break;

			current_scope_index.indices.push_back(this->index.indices[i]);
			i += 1;
		}

		// If loop did not run then this scope is the global scope (special case)
		if (i == 0)
		{
			auto it = types.find(type_name);
			if (it != types.end())
				return result<type>{ it->second };
		}

		// Type not found => not visible in current scope
		return result<type>{};
	}

	result<symbol> scope::find_symbol(const std::string& symbol_name, scope& global_scope)
	{
		assume(this->name != "*___*", "Scope name was \"*___*\"");

		scope_index current_scope_index = global_scope.index;
		u64 i = 0;

		while (true)
		{
			scope* child = global_scope.get_child(current_scope_index);

			auto it = child->symbols.find(symbol_name);
			if (it != child->symbols.end())
			{
				//std::cout << symbol_name << std::endl;
				return result<symbol>{ it->second };
			}

			if (i == this->index.indices.size())
				break;

			current_scope_index.indices.push_back(this->index.indices[i]);
			i += 1;
		}

		// If loop did not run then this scope is the global scope (special case)
		if (i == 0)
		{
			auto it = symbols.find(symbol_name);
			if (it != symbols.end())
			{
				//std::cout << symbol_name << std::endl;
				return result<symbol>{ it->second };
			}
			//std::cout << symbol_name << std::endl;
		}

		// Symbol not found => not visible in current scope
		return result<symbol>{};
	}
}