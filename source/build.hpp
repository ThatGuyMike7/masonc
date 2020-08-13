#ifndef $_MASONC_BUILD_HPP_$
#define $_MASONC_BUILD_HPP_$

#include <common.hpp>
#include <io.hpp>

#include <vector>
#include <unordered_set>
#include <string>

namespace masonc
{
    // Build a list of sources into an object file.
    void build_object(std::vector<path> sources,
        std::unordered_set<std::string> additional_extensions = std::unordered_set<std::string>{});

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