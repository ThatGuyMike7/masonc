#ifndef MASONC_LOCATION_HPP
#define MASONC_LOCATION_HPP

#include <common.hpp>

namespace masonc::lexer
{
    struct token_location
    {
        // A single token can only be on a single line.
        u64 line_number;

        u64 start_column;
        u64 end_column;
    };
}

#endif