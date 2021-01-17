#ifndef MASONC_MODULE_HANDLE_HPP
#define MASONC_MODULE_HANDLE_HPP

#include <common.hpp>

namespace masonc
{
    // Index associated with module name, module struct and AST as stored in
    // the three containers in "masonc::parser::parser_instance_output".
    using mod_handle = u64;
}

#endif