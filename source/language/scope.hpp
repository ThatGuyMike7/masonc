#ifndef $_MASONC_SCOPE_HPP_$
#define $_MASONC_SCOPE_HPP_$

#include <common.hpp>
#include <type.hpp>
#include <symbol.hpp>
#include <containers.hpp>

#include <vector>
#include <optional>

namespace masonc
{
    // Indices on each level of the scope tree, starting at the package scope
    // which is an empty vector. The first element refers to a scope at the first level,
    // e.g. a top-level function scope, and it goes deeper with each subsequent index.
    using scope_index = std::vector<u64>;

    struct package;
    struct scope
    {
        // Usually set by "add_child".
        scope_index index;

        // Package in which this scope is defined.
        // Usually set by "add_child", unless this is a top-level package scope.
        package* package;

        // Variable names, function names, type names, and so on.
        cstring_unordered_set symbols;

        std::vector<scope> children;

        // Returns "nullptr" if the scope is unnamed.
        const char* name();

        // If this scope already has a name, this function does nothing.
        // Otherwise, give it a name.
        void set_name(const char* name, u16 name_length);

        // Retrieve this scope's parents ordered from top to bottom (package scope until this scope).
        std::vector<scope*> parents();

        // Add and initialize a child scope, and return a copy of its "scope_index".
        scope_index add_child(const scope& child);
        scope* get_child(const scope_index& index);

        // Returns false if the symbol is already defined in this scope.
        bool add_symbol(symbol element);

        // Search for a symbol from bottom to top (this scope until package scope),
        // including search in all imported packages.
        // "offset" can be used to skip a number of scopes from the bottom.
        bool find_symbol(symbol element, u64 offset = 0);

    private:
        // Optional name handle for named scopes.
        std::optional<symbol_handle> name_handle;

        // Whether or not a specific symbol is defined in this scope.
        bool is_symbol_defined(symbol element);
    };
}

#endif