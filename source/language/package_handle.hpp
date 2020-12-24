#ifndef $_MASON_PACKAGE_HANDLE_HPP_$
#define $_MASON_PACKAGE_HANDLE_HPP_$

#include <common.hpp>

namespace masonc
{
    // Index associated with package name, package struct and AST as stored in
    // the three containers in "masonc::parser::parser_instance_output".
    using package_handle = u64;
}

#endif