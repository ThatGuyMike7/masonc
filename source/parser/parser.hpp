#ifndef MASONC_PARSER_HPP
#define MASONC_PARSER_HPP

#include <parser_expressions.hpp>
#include <common.hpp>
#include <location.hpp>
#include <lexer.hpp>
#include <symbol.hpp>
#include <scope.hpp>
#include <message.hpp>
#include <mod.hpp>
#include <mod_handle.hpp>
#include <containers.hpp>

#include <string>
#include <vector>
#include <memory>
#include <optional>

namespace masonc::parser
{
    enum parse_context : u8
    {
        CONTEXT_NONE,
        CONTEXT_STATEMENT
    };

    struct parser_instance_output
    {
        masonc::lexer::lexer_instance_output lexer_output;

        std::string module_name;
        mod file_module;
        std::vector<expression> AST;

        // All heap-allocated expressions - the indirection is needed to
        // avoid circular references in some cases.
        std::vector<expression*> delete_list_expressions;

        message_list messages;

        // Release all heap-allocated expressions.
        // This renders the AST unsafe to access, call at the very end of the build process
        // once the AST is not needed anymore.
        void free();
        void print_expressions();
        std::string format_expression(const expression& expr, u64 level = 0);
    };

    struct parser_instance
    {
        // "parser_output.lexer_output" is expected to have no errors.
        parser_instance(parser_instance_output* parser_output);

    private:
        parser_instance_output* parser_output;

        scope_index current_scope_index;
        u64 token_index = 0;

        bool done = false;

        // Drives the parser by parsing top-level expressions which
        // in turn parse their own expressions and so on.
        void drive();

        masonc::lexer::lexer_instance_output* lexer_output();
        scope* current_scope();

        // Whether or not a module declaration has been parsed.
        bool module_declaration_exists();

        // Sets module with specified name to be current, or adds a new module if it doesn't exist.
        void set_module(const char* module_name, u16 module_name_length);
        void set_module(const std::string& module_name);

        void eat(u64 count = 1);
        std::optional<masonc::lexer::token*> peek_token();

        // If a next token does not exist, the parser is marked as "done",
        // a parse error is generated and the result is empty.
        std::optional<masonc::lexer::token*> expect_any();

        // If a next token does not exist, the parser is marked as "done",
        // a parse error is generated and the result is empty.
        //
        // If the next token is not an identifier, a parse error is generated and the result is empty.
        //
        // Otherwise, the token is eaten and the result is valid.
        std::optional<masonc::lexer::token*> expect_identifier();

        // If a next token does not exist, the parser is marked as "done",
        // a parse error is generated and the result is empty.
        //
        // If the next token is not a character matching the argument,
        // a parse error is generated and the result is empty.
        //
        // Otherwise, the token is eaten and the result is valid.
        std::optional<masonc::lexer::token*> expect(char c);

        // If a next token does not exist, the parser is marked as "done",
        // a parse error is generated and the result is empty.
        //
        // If the next token is not an identifier matching the argument,
        // a parse error is generated and the result is empty.
        //
        // Otherwise, the token is eaten and the result is valid.
        std::optional<masonc::lexer::token*> expect(const std::string& identifier);

        const char* identifier_at(const masonc::lexer::token& identifier_token);
        const char* integer_at(const masonc::lexer::token& integer_token);
        const char* decimal_at(const masonc::lexer::token& decimal_token);
        const char* string_at(const masonc::lexer::token& string_token);

        // Assumes that the index is in range.
        masonc::lexer::token_location* get_token_location(u64 token_index);

        // Reports an error at the last token.
        void report_parse_error(const std::string& msg);

        // Reports an error at a specific token.
        void report_parse_error_at(const std::string& msg, u64 token_index);

        // Jumps to the next statement.
        void recover();

        // Guarantees that the next token exists and is an identifier, unless an error occured.
        std::optional<u8> parse_specifiers();

        // top-level expression
        // := expression_variable_declaration |
        //	  expression_procedure_prototype  |
        //	  expression_procedure_definition |
        //	  expression_module_declaration  |
        //	  expression_module_import
        std::optional<expression> parse_top_level();

        // statement (statements allowed in procedure bodies)
        // := expression_variable_declaration |
        //    expression_procedure_call		  |
        //    assignment					  |
        //	  "return" expression
        std::optional<expression> parse_statement();

        // expression
        // := expression_primary | expression_binary

        // Parse primary or binary expressions
        std::optional<expression> parse_expression(parse_context context);

        // expression_primary
        // := expression_reference | expression_literal | expression_procedure_call |
        //    expression_unary
        std::optional<expression> parse_primary(parse_context context);

        // Parses right-hand side of a binary expression and returns the whole binary expression
        std::optional<expression> parse_binary(parse_context context,
            const expression& left, const binary_operator* op);

        // Parses either expression_primary which would ignore the parentheses,
        // or binary expression encased in parentheses, returning expression_parentheses
        std::optional<expression> parse_parentheses(parse_context context);

        std::optional<expression> parse_number_literal(parse_context context,
            const char* number, number_type type);

        std::optional<expression> parse_string_literal(parse_context context, const char* str);

        std::optional<expression> parse_reference(parse_context context,
            symbol_handle identifier_handle);

        std::optional<expression> parse_variable_declaration(parse_context context,
            symbol_handle name_handle, u8 specifiers);

        std::optional<expression> parse_call(parse_context context, symbol_handle name_handle);

        std::vector<expression> parse_argument_list();

        // Returns either Expression_Procedure_Prototype or Expression_Procedure_Definition
        std::optional<expression> parse_procedure();
        std::optional<expression> parse_procedure_body(const expression_procedure_prototype& prototype);

        std::optional<expression> parse_module_declaration();
        std::optional<expression> parse_module_import();
    };

    // Returns `expression_binary` from either `expression_binary` or `expression_parentheses`.
    // Any other passed expression type will return a null pointer.
    expression_binary* get_binary_expression(expression* expr);
}

#endif