#include <scope.hpp>

#include <log.hpp>
#include <common.hpp>
#include <mod.hpp>

#include <iostream>
#include <optional>

namespace masonc
{
    scope_index scope::index()
    {
        return m_index;
    }

    const mod& scope::get_module()
    {
        return *m_module;
    }

    const char* scope::name()
    {
        if (name_handle)
            return m_module->scope_names.at(name_handle.value());
        else
            return nullptr;
    }

    void scope::set_name(const char* name, u16 name_length)
    {
        if (name_handle)
            return;

        name_handle = m_module->scope_names.copy_back(name, name_length);
    }

    // TODO: Instead of doing this, just keep a vector of pointers to parents in the scope.
    std::vector<scope*> scope::parents()
    {
        scope* current_scope = &m_module->module_scope;
        std::vector<scope*> parents = { current_scope };

        for (u64 i = 0; i < m_index.size(); i += 1) {
            current_scope = &current_scope->children[m_index[i]];
            parents.push_back(current_scope);
        }

        return parents;
    }

    scope_index scope::add_child(const scope& child)
    {
        u64 added_child_index = children.size();
        scope* added_child = &children.emplace_back(child);

        added_child->m_index = m_index;
        added_child->m_index.push_back(added_child_index);

        added_child->m_module = m_module;

        return added_child->m_index;
    }

    scope* scope::get_child(const scope_index& index)
    {
        scope* child = this;

        for (u64 i = child->m_index.size(); i < index.size(); i += 1) {
            child = &child->children[index[i]];
        }

        return child;
    }

    bool scope::add_symbol(symbol element)
    {
        if (is_symbol_defined(element))
            return false;

        symbols.insert(element);

        return true;
    }

    bool scope::find_symbol(symbol element, u64 offset)
    {
        auto parent_scopes = parents();

        // TODO: Look at imported modules as well.

        if (parent_scopes.size() <= offset)
            return false;

        for (u64 i = parent_scopes.size() - offset; i >= 0; i -= 1) {
            scope* current_parent_scope = parent_scopes[i];
            auto symbol_search = current_parent_scope->symbols.find(element);

            if (symbol_search != current_parent_scope->symbols.end())
                return true;
        }

        return false;
    }

    bool scope::is_symbol_defined(symbol element)
    {
        return (symbols.find(element) != symbols.end());
        //return (symbols_lookup.find(element) != symbols_lookup.end());
    }
}