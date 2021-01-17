#ifndef $_MASONC_LINKER_HPP_$
#define $_MASONC_LINKER_HPP_$

#include <location.hpp>
#include <parser.hpp>
#include <mod.hpp>
#include <scope.hpp>
#include <message.hpp>
//#include <dependency_list.hpp>

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
        mod* module_from_name(const char* module_name);

        // "parser_outputs" are expected to have no errors and
        // "linker_output" is expected to be allocated and empty.
        void link(std::vector<masonc::parser::parser_instance_output>* parser_outputs,
            linker_output* linker_output);

    private:
        linker_output* linker_output;
        //dependency_graph<mod*> module_graph;

        //masonc::parser::parser_instance_output* parser_output();

        void report_link_error(const std::string& msg, const masonc::lexer::token_location& location);
    };
}

#endif