#ifndef MASONC_BUILD_HPP
#define MASONC_BUILD_HPP

#include <common.hpp>
#include <io.hpp>

#include <robin_hood.hpp>

#include <vector>
#include <string>

namespace masonc
{
    // Build a list of sources into an object file.
    void build_object(std::vector<path> sources,
        robin_hood::unordered_set<std::string> additional_extensions = robin_hood::unordered_set<std::string>{});

    enum class build_stage : u8
    {
        UNSET,
        LEXER,
        PARSER,
        LINKER,
        CODE_GENERATOR
    };

    // Returns the name of a build stage as a string
    const std::string build_stage_name(build_stage stage);
}

#endif