#ifndef $_MASON_PACKAGE_HPP_$
#define $_MASON_PACKAGE_HPP_$

#include <common.hpp>
#include <scope.hpp>

#include <string>
#include <unordered_map>
#include <set>

namespace masonc
{
    inline scope PACKAGE_SCOPE_TEMPLATE{ "package_scope" };

    // Insert language-defined types into "PACKAGE_SCOPE_TEMPLATE".
    void initialize_package_scope_template();

    // Receive the module name from a package string,
    // e.g. "core" from "core.io.streams".
    std::string module_name_from_package_name(const std::string& package_name);

    struct package_import
    {
        std::string package_name;

        bool operator< (const package_import& rhs) const
        {
            return package_name < rhs.package_name;
        }
    };

    struct package
    {
        std::set<package_import> imports;
        scope package_scope = PACKAGE_SCOPE_TEMPLATE;
    };

    // Package names associated with package structures.
    using package_map = std::unordered_map<std::string, package>;
}

#endif