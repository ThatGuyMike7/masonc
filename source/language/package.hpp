#ifndef $_MASON_PACKAGE_HPP_$
#define $_MASON_PACKAGE_HPP_$

#include <common.hpp>
#include <package_handle.hpp>
#include <scope.hpp>
#include <parser_expressions.hpp>
#include <containers.hpp>

#include <string>
#include <vector>

namespace masonc
{
    inline scope PACKAGE_SCOPE_TEMPLATE;

    // Insert language-defined types into "PACKAGE_SCOPE_TEMPLATE".
    void initialize_package_scope_template();

    // Receive the module name from a package string,
    // e.g. "core" from "core.io.streams".
    std::string module_name_from_package_name(const std::string& package_name);

    struct package
    {
        cstring_collection package_import_names;

        scope package_scope = PACKAGE_SCOPE_TEMPLATE;
        cstring_collection scope_names;
    };
}

#endif