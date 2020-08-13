#ifndef $_MASONC_PARSER_HPP_$
#define $_MASONC_PARSER_HPP_$

#include <parser_expressions.hpp>
#include <common.hpp>
#include <lexer.hpp>
#include <symbol.hpp>
#include <scope.hpp>
#include <message.hpp>
#include <package.hpp>

#include <string>
#include <vector>
#include <memory>
#include <optional>

namespace masonc
{
    // Package names associated with ASTs.
    using package_ast_map = std::unordered_map<std::string, std::vector<expression>>;

    enum parse_context : u8
    {
        CONTEXT_NONE,
        CONTEXT_STATEMENT
    };

    struct parser_output
    {
        package_map packages;

        // Each package defined in the parser's input gets its own AST.
        // After all parsers finished, ASTs of equal packages will be merged in the pre-linker.
        package_ast_map asts;

        message_list messages;
    };

    struct parser
    {
        // "input" is expected to have no errors,
        // "output" is expected to be allocated and empty.
        void parse(lexer_output* input, parser_output* output);

        void print_expressions();
        std::string format_expression(const expression& expr, u64 level = 0);

    private:
        lexer_output* input;
        parser_output* output;

        // "nullptr" when no package declaration was parsed.
        package* current_package;
        std::vector<expression>* current_ast;
        std::string current_package_name;

        scope_index current_scope_index;
        u64 token_index;

        bool done;

        // Drives the parser by parsing top-level expressions which
        // in turn parse their own expressions and so on.
        void drive();

        scope* current_scope();

        // Set package with specified name to be current, or add a new package if it doesn't exist.
        void set_package(const std::string& package_name);

        void eat(u64 count = 1);
        std::optional<token*> peek_token();

        // If a next token does not exist, the parser is marked as "done",
        // a parse error is generated and the result is empty.
        std::optional<token*> expect_any();

        // If a next token does not exist, the parser is marked as "done",
        // a parse error is generated and the result is empty.
        //
        // If the next token is not an identifier, a parse error is generated and the result is empty.
        //
        // Otherwise, the token is eaten and the result is valid.
        std::optional<token*> expect_identifier();

        // If a next token does not exist, the parser is marked as "done",
        // a parse error is generated and the result is empty.
        //
        // If the next token is not a character matching the argument,
        // a parse error is generated and the result is empty.
        //
        // Otherwise, the token is eaten and the result is valid.
        std::optional<token*> expect(char c);

        // If a next token does not exist, the parser is marked as "done",
        // a parse error is generated and the result is empty.
        //
        // If the next token is not an identifier matching the argument,
        // a parse error is generated and the result is empty.
        //
        // Otherwise, the token is eaten and the result is valid.
        std::optional<token*> expect(const std::string& identifier);

        const std::string& identifier_at(const token& identifier_token);
        const std::string& integer_at(const token& integer_token);
        const std::string& decimal_at(const token& decimal_token);
        const std::string& string_at(const token& string_token);

        // Assumes that the index is in range.
        token_location* get_token_location(u64 token_index);

        // Report an error at the last token.
        void report_parse_error(const std::string& msg);

        // Report an error at a specific token.
        void report_parse_error_at(const std::string& msg, u64 token_index);

        // Jump to the next statement.
        void recover();

        // Guarantees that the next token exists and is an identifier, unless an error occured.
        std::optional<u8> parse_specifiers();

        // top-level expression
        // := expression_variable_declaration |
        //	  expression_procedure_prototype  |
        //	  expression_procedure_definition |
        //	  expression_package_declaration  |
        //	  expression_package_import
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

        // Parse either expression_primary which would ignore the parentheses,
        // or binary expression encased in parentheses, returning expression_parentheses
        std::optional<expression> parse_parentheses(parse_context context);

        std::optional<expression> parse_number_literal(parse_context context,
            const std::string& number, number_type type);

        std::optional<expression> parse_string_literal(parse_context context, const std::string& str);
        std::optional<expression> parse_reference(parse_context context, const std::string& identifier);

        std::optional<expression> parse_variable_declaration(parse_context context,
            const std::string& variable_name, u8 specifiers);

        std::optional<expression> parse_call(parse_context context, const std::string& procedure_name);
        std::vector<expression> parse_argument_list();

        // Returns either Expression_Procedure_Prototype or Expression_Procedure_Definition
        std::optional<expression> parse_procedure();
        std::optional<expression> parse_procedure_body(const expression_procedure_prototype& prototype);

        std::optional<expression> parse_package_declaration();
        std::optional<expression> parse_package_import();
    };

    // Returns `expression_binary` from either `expression_binary` or `expression_parentheses`.
    // Any other passed expression type will return a null pointer.
    expression_binary* get_binary_expression(expression* expr);
}

#endif