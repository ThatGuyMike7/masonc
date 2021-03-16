#ifndef MASONC_MODULE_HPP
#define MASONC_MODULE_HPP

#include <scope.hpp>
#include <containers.hpp>

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