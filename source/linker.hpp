#ifndef $_MASONC_LINKER_HPP_$
#define $_MASONC_LINKER_HPP_$

#include <location.hpp>
#include <parser.hpp>
#include <package.hpp>
#include <scope.hpp>
#include <message.hpp>
#include <dependency_graph.hpp>

#include <vector>
#include <optional>

namespace masonc::linker
{
    struct linker_output
    {
        masonc::parser::parser_output* parser_output;

        message_list messages;
    };

    // The linker resolves package and code dependencies for out-of-order compilation,
    // any circular dependencies are reported as errors.
    // It thus also checks if symbols and types are visible from where they are used and
    // if they are used correctly.
    //
    // The linker requires that the lexer and parser stages finished completely.
    struct linker
    {
        // Returns "nullptr" if package is not defined.
        package* package_from_name(const char* package_name);

        // "parser_output" is expected to have no errors and
        // "linker_output" is expected to be allocated and empty.
        void link(masonc::parser::parser_output* parser_output, linker_output* linker_output);

    private:
        linker_output* linker_output;
        //dependency_graph<package*> package_graph;

        masonc::parser::parser_output* parser_output();
    };
}

#endif