#ifndef $_MASON_MODULE_HANDLE_HPP_$
#define $_MASON_MODULE_HANDLE_HPP_$

#include <common.hpp>

namespace masonc
{
    // Index associated with module name, module struct and AST as stored in
    // the three containers in "masonc::parser::parser_instance_output".
    using module_handle = u64;
}

#endif