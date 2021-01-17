#include <mod.hpp>

#include <language.hpp>

namespace masonc
{
    void initialize_module_scope_template()
    {
        MODULE_SCOPE_TEMPLATE.add_symbol(TYPE_VOID);

        MODULE_SCOPE_TEMPLATE.add_symbol(TYPE_BOOL);
        MODULE_SCOPE_TEMPLATE.add_symbol(TYPE_CHAR);
        MODULE_SCOPE_TEMPLATE.add_symbol(TYPE_U8);
        MODULE_SCOPE_TEMPLATE.add_symbol(TYPE_S8);

        MODULE_SCOPE_TEMPLATE.add_symbol(TYPE_U16);
        MODULE_SCOPE_TEMPLATE.add_symbol(TYPE_S16);

        MODULE_SCOPE_TEMPLATE.add_symbol(TYPE_U32);
        MODULE_SCOPE_TEMPLATE.add_symbol(TYPE_S32);
        MODULE_SCOPE_TEMPLATE.add_symbol(TYPE_F32);

        MODULE_SCOPE_TEMPLATE.add_symbol(TYPE_U64);
        MODULE_SCOPE_TEMPLATE.add_symbol(TYPE_S64);
        MODULE_SCOPE_TEMPLATE.add_symbol(TYPE_F64);
    }
}