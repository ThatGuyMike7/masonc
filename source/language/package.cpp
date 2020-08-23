#include <package.hpp>

#include <language.hpp>

namespace masonc
{
    void initialize_package_scope_template()
    {
        PACKAGE_SCOPE_TEMPLATE.add_symbol(TYPE_VOID);

        PACKAGE_SCOPE_TEMPLATE.add_symbol(TYPE_BOOL);
        PACKAGE_SCOPE_TEMPLATE.add_symbol(TYPE_CHAR);
        PACKAGE_SCOPE_TEMPLATE.add_symbol(TYPE_U8);
        PACKAGE_SCOPE_TEMPLATE.add_symbol(TYPE_S8);

        PACKAGE_SCOPE_TEMPLATE.add_symbol(TYPE_U16);
        PACKAGE_SCOPE_TEMPLATE.add_symbol(TYPE_S16);

        PACKAGE_SCOPE_TEMPLATE.add_symbol(TYPE_U32);
        PACKAGE_SCOPE_TEMPLATE.add_symbol(TYPE_S32);
        PACKAGE_SCOPE_TEMPLATE.add_symbol(TYPE_F32);

        PACKAGE_SCOPE_TEMPLATE.add_symbol(TYPE_U64);
        PACKAGE_SCOPE_TEMPLATE.add_symbol(TYPE_S64);
        PACKAGE_SCOPE_TEMPLATE.add_symbol(TYPE_F64);
    }

    std::string module_name_from_package_name(const std::string& package_name)
    {
        size_t dot_index = package_name.find('.');

        if (dot_index == std::string::npos)
            return package_name;
        else
            return package_name.substr(0, dot_index);
    }
}