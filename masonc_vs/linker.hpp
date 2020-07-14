#ifndef $_MASONC_LINKER_HPP_$
#define $_MASONC_LINKER_HPP_$

#include "scope.hpp"
#include "message.hpp"
#include "parser.hpp"

namespace masonc
{
    struct pre_linker_output
    {
        scope merged_global_scope;
        message_list messages;
    };
    
    // The pre-linker's job is to merge all package scope instances
    // from all the parsers together (and maybe also the ASTs).
    // The lexer, parser and pre-linker stages should run in parallel.
    struct pre_linker
    {
        // TODO: Implement this
        void merge(parser_output* input);
        
    private:
        parser_output* input;
        pre_linker_output* output;
        
        // Input scopes must be package-level
        void merge_package_scopes(scope& destination, scope& source);

        // TODO: Implement this
        void merge_packages();
    };
    
    struct linker_output
    {
        message_list messages;
    };
    
    // The linker's job is to check if used symbols and types are correctly defined
    // and visible (and maybe also change the order to prepare for code generation).
    // The linker requires that the lexer, parser and pre-linker stages finished.
	struct linker
	{
        // TODO: Implement this
        void link(pre_linker_output* input);
        
	private:
        pre_linker_output* input;
        linker_output* output;
	};
}

#endif