#ifndef $_MASON_MODULE_HPP_$
#define $_MASON_MODULE_HPP_$

#include <common.hpp>
#include <mod_handle.hpp>
#include <scope.hpp>
#include <parser_expressions.hpp>
#include <containers.hpp>

#include <string>
#include <vector>

namespace masonc
{
    inline scope MODULE_SCOPE_TEMPLATE;

    // Insert language-defined types into "MODULE_SCOPE_TEMPLATE".
    void initialize_module_scope_template();

    struct mod
    {
        cstring_collection module_import_names;

        scope module_scope = MODULE_SCOPE_TEMPLATE;
        cstring_collection scope_names;
    };
}

#endif