#ifndef MASONC_BUILD_STAGE_HPP
#define MASONC_BUILD_STAGE_HPP

#include <common.hpp>

namespace masonc
{
    enum class build_stage : u8
    {
        UNSET,
        LEXER,
        PARSER,
        LINKER,
        BYTECODE_GENERATOR,
        CODE_GENERATOR
    };

    // Returns the name of a build stage as a string
    const std::string build_stage_name(build_stage stage);
}

#endif