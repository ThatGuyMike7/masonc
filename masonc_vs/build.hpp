#ifndef $_MASONC_BUILD_HPP_$
#define $_MASONC_BUILD_HPP_$

#include "common.hpp"

namespace masonc
{
    enum class build_stage : u8
    {
        UNSET,
    	LEXER,
    	PARSER,
        PRE_LINKER,
    	LINKER,
    	CODE_GENERATOR
    };
    
    // Returns the name of a build stage as a string
    const std::string build_stage_name(build_stage stage);
}

#endif