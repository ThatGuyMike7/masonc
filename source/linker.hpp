#ifndef MASONC_LINKER_HPP
#define MASONC_LINKER_HPP

#include <location.hpp>
#include <lexer.hpp>
#include <parser.hpp>
#include <mod.hpp>
#include <mod_handle.hpp>
#include <scope.hpp>
#include <message.hpp>
#include <dependency_list.hpp>

#include <string>

namespace masonc::linker
{
    struct linker_output
    {
        std::vector<masonc::parser::parser_instance_output>* parser_outputs;

        message_list messages;
    };

    // The linker resolves module dependencies, any circular dependencies are reported as errors.
    //
    // The linker requires that the lexer and parser stages finished completely.
    struct linker
    {
        // Returns "nullptr" if module is not defined.
        // This function can only be called after "link" was called.
        //mod* module_from_name(const char* module_name);

        // "parser_outputs" are expected to have no errors and
        // "linker_output" is expected to be allocated and empty.
        void link(std::vector<masonc::parser::parser_instance_output>* parser_outputs,
            linker_output* linker_output);

    private:
        linker_output* linker_output;
        //dependency_graph<mod*> module_graph;

        void report_link_error(const std::string& msg, const masonc::lexer::token_location& location);
    };
}

#endif