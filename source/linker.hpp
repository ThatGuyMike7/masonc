#ifndef $_MASONC_LINKER_HPP_$
#define $_MASONC_LINKER_HPP_$

#include <scope.hpp>
#include <message.hpp>
#include <parser.hpp>

namespace masonc
{
    struct linker_output
    {
        message_list messages;
    };

    // The linker checks if used symbols and types are correctly defined
    // and visible (and maybe also change the order to prepare for code generation).
    // The linker requires that the lexer, parser and pre-linker stages finished.
    struct linker
    {
        // TODO: Implement this.
        void link(parser_output* input);

    private:
        parser_output* input;
        linker_output* output;
    };
}

#endif