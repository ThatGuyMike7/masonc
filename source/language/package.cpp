#include <package.hpp>

#include <lang.hpp>

namespace masonc
{
    void initialize_package_scope_template()
    {
        PACKAGE_SCOPE_TEMPLATE.add_type(type{ TYPE_VOID }, PACKAGE_SCOPE_TEMPLATE);

        PACKAGE_SCOPE_TEMPLATE.add_type(type{ TYPE_BOOL }, PACKAGE_SCOPE_TEMPLATE);
        PACKAGE_SCOPE_TEMPLATE.add_type(type{ TYPE_CHAR }, PACKAGE_SCOPE_TEMPLATE);
        PACKAGE_SCOPE_TEMPLATE.add_type(type{ TYPE_U8 }, PACKAGE_SCOPE_TEMPLATE);
        PACKAGE_SCOPE_TEMPLATE.add_type(type{ TYPE_S8 }, PACKAGE_SCOPE_TEMPLATE);

        PACKAGE_SCOPE_TEMPLATE.add_type(type{ TYPE_U16 }, PACKAGE_SCOPE_TEMPLATE);
        PACKAGE_SCOPE_TEMPLATE.add_type(type{ TYPE_S16 }, PACKAGE_SCOPE_TEMPLATE);

        PACKAGE_SCOPE_TEMPLATE.add_type(type{ TYPE_U32 }, PACKAGE_SCOPE_TEMPLATE);
        PACKAGE_SCOPE_TEMPLATE.add_type(type{ TYPE_S32 }, PACKAGE_SCOPE_TEMPLATE);
        PACKAGE_SCOPE_TEMPLATE.add_type(type{ TYPE_F32 }, PACKAGE_SCOPE_TEMPLATE);

        PACKAGE_SCOPE_TEMPLATE.add_type(type{ TYPE_U64 }, PACKAGE_SCOPE_TEMPLATE);
        PACKAGE_SCOPE_TEMPLATE.add_type(type{ TYPE_S64 }, PACKAGE_SCOPE_TEMPLATE);
        PACKAGE_SCOPE_TEMPLATE.add_type(type{ TYPE_F64 }, PACKAGE_SCOPE_TEMPLATE);
    }

    std::string module_name_from_package_name(const std::string& package_name)
    {
        size_t dot_index = package_name.find('.');

        if(dot_index == std::string::npos)
            return package_name;
        else
            return package_name.substr(0, dot_index);
    }
}