#include "parser.hpp"
#include "common.hpp"
#include "lang.hpp"
#include "log.hpp"
#include "timer.hpp"
#include "lexer.hpp"

#include <iostream>
#include <optional>

namespace masonc
{
    void parser::parse(lexer_output* input, parser_output* output)
    {
        this->input = input;
        this->output = output;

        this->current_package = nullptr;
        this->current_ast = nullptr;
        this->current_package_name = "";

        this->token_index = 0;

        this->done = false;

        // The first statement must be a package declaration.
        auto package_identifier_result = expect("package");
        if (!package_identifier_result) {
            return;
        }

        auto package_declaration_result = parse_package_declaration();
        if (!package_declaration_result/*current_package == nullptr*/) {
            return;
        }

        current_ast->push_back(package_declaration_result.value());

        // Drive the parser.
        drive();
    }
    
    std::string parser::format_expression(const expression& expr, u64 level)
    {
        std::string message;
        for (u64 i = 0; i < level; i += 1) {
            message += "    ";
        }

        // This is safe because no matter the current variant,
        // it is guaranteed to be at the same address
        // (and type is the first member in tagged expression struct).
        switch (expr.value.empty.type) {
            default:
                message += "Unknown Expression";
                break;
            
            case EXPR_EMPTY:
                message += "Empty Expression";
                break;
            
            case EXPR_UNARY:
                message += "Unary Expression: Op_Code='"
                        + std::to_string(expr.value.unary.value.op_code) + "'"
                        + "\n" + format_expression(*expr.value.unary.value.expr, level + 1);
                break;
        
            case EXPR_BINARY:
                message += "Binary Expression: Op_Code='"
                        + std::to_string(expr.value.binary.value.op->op_code) + "'"
                        + "\n" + format_expression(*expr.value.binary.value.left, level + 1)
                        + "\n" + format_expression(*expr.value.binary.value.right, level + 1);
                break;
            
            case EXPR_PARENTHESES:
                message += "Parentheses Expression: Op_Code='"
                        + std::to_string(expr.value.parentheses.value.expr.op->op_code) + "'"
                        + "\n" + format_expression(*expr.value.parentheses.value.expr.left, level + 1)
                        + "\n" + format_expression(*expr.value.parentheses.value.expr.right, level + 1);
                break;
            
            case EXPR_NUMBER_LITERAL:
                message += "Number Literal: Value='" + expr.value.number.value.value + "'";
                break;
            
            case EXPR_STRING_LITERAL:
                message += "String Literal: Value='" + expr.value.str.value.value + "'";
                break;
            
            case EXPR_REFERENCE:
                message += "Reference: Name='" + expr.value.reference.value.name.name + "'";
                break;
            
            case EXPR_VAR_DECLARATION:
                message += "Variable Declaration: Name='"
                        + expr.value.variable_declaration.value.name.name + "' Type='";
            
                if(expr.value.variable_declaration.value.is_pointer)
                    message += "^";
            
                message += expr.value.variable_declaration.value.type_name + "'";
            
                if(expr.value.variable_declaration.value.specifiers != SPECIFIER_NONE) {
                    message += " Specifiers='";
                
                    if(expr.value.variable_declaration.value.specifiers & SPECIFIER_CONST)
                        message += "const ";
                
                    if(expr.value.variable_declaration.value.specifiers & SPECIFIER_MUT)
                        message += "mut ";
                
                    message += "'";
                }
            
                break;
            
            case EXPR_PROC_PROTOTYPE:
                message += "Procedure Prototype Expression: Name='"
                        + expr.value.procedure_prototype.value.name.name
                        + "' Return Type='" + expr.value.procedure_prototype.value.return_type_name
                        + "'";

                if (expr.value.procedure_prototype.value.argument_list.size() > 0)
                    message += "\nArgument List: ";

                for (u64 i = 0; i < expr.value.procedure_prototype.value.argument_list.size(); i += 1) {
                    message += "\n" + format_expression(
                        expr.value.procedure_prototype.value.argument_list[i], level + 1
                    );
                }
                break;
            
            case EXPR_PROC_DEFINITION:
                message += format_expression(
                    expression{ expr.value.procedure_definition.value.prototype }, level
                );

                if (expr.value.procedure_definition.value.body.size() > 0)
                    message += "\nBody: ";

                for (u64 i = 0; i < expr.value.procedure_definition.value.body.size(); i += 1) {
                    message += "\n" + format_expression(
                        expr.value.procedure_definition.value.body[i], level + 1);
                }
                break;
            
            case EXPR_PROC_CALL:
                message += "Procedure Call Expression: Name='"
                    + expr.value.procedure_call.value.name.name
                    + "'";

                for (u64 i = 0; i < expr.value.procedure_call.value.argument_list.size(); i += 1) {
                    message += "\n" + format_expression(
                        expr.value.procedure_call.value.argument_list[i], level + 1
                    );
                }
                break;
            
            case EXPR_PACKAGE_DECLARATION:
                message += "Package Declaration: Name='"
                        + expr.value.package_declaration.value.package_name + "'";
                break;
            
            case EXPR_PACKAGE_IMPORT:
                message += "Package Import: Name='"
                        + expr.value.package_import.value.package_name + "'";
                break;
        }

        return message;
    }
    
    void parser::print_expressions()
    {
        for(auto ast_it = output->asts.begin(); ast_it != output->asts.end(); ast_it++) {
            std::vector<expression>& ast = ast_it->second;
            
            for (u64 i = 0; i < ast.size(); i += 1) {
                std::cout << format_expression(ast[i]) << std::endl;
            }
        }
    }
    
    void parser::drive()
    {
        while(true) {
            auto top_level_expression = parse_top_level();
            if (top_level_expression)
                current_ast->push_back(top_level_expression.value());
            
            // TODO: Change all "parse_" methods to return "std::optional<expression>"
            //       as well as use the new "_at" and "expect" methods.
            
            // Reached the end of the token stream.
            if (done)
                break;
        }
    }
    
    scope* parser::current_scope()
    {
        return current_package->package_scope.get_child(current_scope_index);
    }
    
    void parser::set_package(const std::string& package_name)
    {
        current_package_name = package_name;
        
        auto search_package = output->packages.find(package_name);
        auto search_ast = output->asts.find(package_name);
        
        if (search_package != output->packages.end() && search_ast != output->asts.end()) {
            // Found the package and AST
            current_package = &search_package->second;
            current_ast = &search_ast->second;
        }
        else {
            // Create new package and AST
            auto inserted_package = output->packages.insert({ package_name, package{} });
            auto inserted_ast = output->asts.insert({ package_name, std::vector<expression>{} });
            
            assume(inserted_package.second && inserted_ast.second,
                "either package exists and AST does not or AST exists and package does not");
            
            current_package = &inserted_package.first->second;
            current_ast = &inserted_ast.first->second;

            try {
                // Guess how many tokens will end up being 1 expression on average to
                // avoid reallocations.
                current_ast->reserve(input->tokens.size() / 10 + 32);
            }
            catch (...) {
                log_warning("could not reserve space for AST vector");
            }
        }
        
        current_package->package_scope.name = current_package_name;
        current_scope_index = current_package->package_scope.index;
    }

    void parser::eat(u64 count)
    {
        token_index += count;
    }
    
    std::optional<token*> parser::peek_token()
    {
        if(token_index < input->tokens.size()) {
            std::optional<token*> result{ &input->tokens[token_index] };
            return result;
        }
        
        return std::optional<token*>{};
    }
    
    std::optional<token*> parser::expect_any()
    {
        auto token_result = peek_token();
        if (!token_result) {
            report_parse_error("Expected a token.");
            done = true;
            return std::optional<token*>{};
        }
        
        eat();
        return token_result.value();
    }
    
    std::optional<token*> parser::expect_identifier()
    {
        auto token_result = peek_token();
        if (!token_result) {
            report_parse_error("Expected a token.");
            done = true;
            return std::optional<token*>{};
        }
        
        if (token_result.value()->type != TOKEN_IDENTIFIER) {
            report_parse_error("Expected an identifier.");
            return std::optional<token*>{};
        }
        
        eat();
        return token_result.value();
    }
    
    std::optional<token*> parser::expect(char c)
    {
        auto token_result = peek_token();
        if (!token_result) {
            report_parse_error("Expected a token.");
            done = true;
            return std::optional<token*>{};
        }
        
        // Assumes that type "char" is signed
        if (token_result.value()->type != c) {
            report_parse_error("Expected \"" + std::string{ c } + "\".");
            return std::optional<token*>{};
        }
        
        eat();
        return token_result.value();
    }
    
    std::optional<token*> parser::expect(const std::string& identifier)
    {
        auto token_result = peek_token();
        if (!token_result) {
            report_parse_error("Expected a token.");
            done = true;
            return std::optional<token*>{};
        }
        
        if (token_result.value()->type != TOKEN_IDENTIFIER ||
            identifier_at(*token_result.value()) != identifier)
        {
            report_parse_error("Expected \"" + identifier + "\".");
            return std::optional<token*>{};
        }
        
        eat();
        return token_result.value();
    }
    
    const std::string& parser::identifier_at(const token& identifier_token)
    {
        assume(input->identifiers.size() > identifier_token.value_index, "value_index is out of range");
        return input->identifiers[identifier_token.value_index];
    }
    
    const std::string& parser::integer_at(const token& integer_token)
    {
        assume(input->integers.size() > integer_token.value_index, "value_index is out of range");
        return input->integers[integer_token.value_index];
    }
    
    const std::string& parser::decimal_at(const token& decimal_token)
    {
        assume(input->decimals.size() > decimal_token.value_index, "value_index is out of range");
        return input->decimals[decimal_token.value_index];
    }
    
    const std::string& parser::string_at(const token& string_token)
    {
        assume(input->strings.size() > string_token.value_index, "value_index is out of range");
        return input->strings[string_token.value_index];
    }
    
    token_location* parser::get_token_location(u64 token_index)
    {
        return &input->locations[token_index];
    }
    
    void parser::report_parse_error(const std::string& msg)
    {
        // TODO: Mark as unlikely.
        if (this->token_index == 0)
            report_parse_error_at(msg, this->token_index);
        else
            report_parse_error_at(msg, this->token_index - 1);
    }
    
    void parser::report_parse_error_at(const std::string& msg, u64 token_index)
    {
        token_location* location = get_token_location(token_index);
        
        output->messages.report_error(msg, build_stage::PARSER,
            location->line_number, location->start_column, location->end_column
        );
    }
    
    void parser::recover()
    {
        while (true) {
            std::optional<token*> token_result = peek_token();
            if (!token_result)
                return;
            
            eat();
            
            if (token_result.value()->type == ';' ||
                token_result.value()->type == '}')
            {
                return;
            }
        }
    }
    
    std::optional<u8> parser::parse_specifiers()
    {
        u8 specifiers = SPECIFIER_NONE;

        while (true) {
            auto token_result = peek_token();
            if (!token_result) {
                done = true;
                return std::nullopt;
            }

            if (token_result.value()->type != TOKEN_IDENTIFIER) {
                report_parse_error("Expected an identifier.");
                recover();

                return std::nullopt;
            }

            auto identifier = identifier_at(*token_result.value());

            if (identifier == "mut") {
                eat();

                if (specifiers & SPECIFIER_MUT) {
                    report_parse_error("Specifier is already defined.");
                    continue;
                }

                specifiers |= SPECIFIER_MUT;
            }
            else if (identifier == "const") {
                eat();

                if (specifiers & SPECIFIER_CONST) {
                    report_parse_error("Specifier is already defined.");
                    continue;
                }

                specifiers |= SPECIFIER_CONST;
            }
            else {
                return std::optional<u8>{ specifiers };
            }
        }
    }
    
    std::optional<expression> parser::parse_top_level()
    {
        std::optional<u8> specifiers_result = parse_specifiers();
        if (!specifiers_result)
            return std::nullopt;

        // Next token is guaranteed to exist and be an identifier.
        auto token_result = peek_token();
        const std::string& identifier = identifier_at(*token_result.value());
        eat();

        if (specifiers_result.value() == SPECIFIER_NONE) {
            if (identifier == "proc")
                return parse_procedure();
            if (identifier == "package")
                return parse_package_declaration();
            if (identifier == "import")
                return parse_package_import();
        }

        // Specifiers are declared and parsed.

        token_result = expect(':');
        if (!token_result) {
            recover();
            return std::nullopt;
        }

        return parse_variable_declaration(CONTEXT_STATEMENT, identifier, specifiers_result.value());
    }

    std::optional<expression> parser::parse_statement()
    {
        std::optional<u8> specifiers_result = parse_specifiers();
        if(!specifiers_result)
            return std::nullopt;
        
        // Next token is guaranteed to exist and be an identifier.
        auto token_result = peek_token();
        const std::string& identifier = identifier_at(*token_result.value());
        eat();

        if (identifier == "return") {
            // Parse return statement.
            return parse_expression(CONTEXT_STATEMENT);
            
            // TODO: Check if type of returned value matches procedure return type.
        }
        
        token_result = peek_token();
        if (!token_result) {
            report_parse_error("Expected a token.");
            done = true;
            return std::nullopt;
        }

        if (specifiers_result.value() == SPECIFIER_NONE &&
            token_result.value()->type == '(')
        {
            eat();
            return parse_call(CONTEXT_STATEMENT, identifier);
        }
        
        // Specifiers are declared or next token is not "(".

        if (token_result.value()->type == ':') {
            eat();  
            return parse_variable_declaration(CONTEXT_STATEMENT, identifier, specifiers_result.value());
        }

        report_parse_error("Unexpected token.");
        recover();
        return std::nullopt;
    }

    std::optional<expression> parser::parse_expression(parse_context context)
    {
        auto primary_result = parse_primary(CONTEXT_NONE);
        if (!primary_result)
            return std::nullopt;
        
        auto token_result = peek_token();
        if (!token_result) {
            report_parse_error("Expected a token.");
            done = true;
            return std::nullopt;
        }

        auto op_result = get_op(token_result.value()->type);
        if (!op_result) {
            // Next token is not a binary operator.

            if (context == CONTEXT_STATEMENT) {
                if (token_result.value()->type != ';') {
                    recover();
                    return std::nullopt;
                }
                else {
                    // Eat the ";".
                    eat();
                }
            }

            return primary_result;
        }
        else {
            // Eat the binary operator.
            eat();
        }

        // Parse right-hand side of binary expression.
        return parse_binary(context, primary_result.value(), op_result.value());
    }
        
    std::optional<expression> parser::parse_primary(parse_context context)
    {
        auto token_result = peek_token();
        if (!token_result) {
            report_parse_error("Expected a token.");
            done = true;
            return std::nullopt;
        }
        
        switch (token_result.value()->type) {
            default: {
                report_parse_error("Unexpected token.");
                recover();
                return std::nullopt;
            }
            case '^':
            case '&': {
                eat();
                auto primary_result = parse_primary(context);
                if (!primary_result)
                    return std::nullopt;

                return expression{
                    expression_unary{
                        // TODO: Delete this later.
                        new expression{ primary_result.value() },
                        token_result.value()->type
                    }
                };
            }
            case TOKEN_IDENTIFIER: {
                eat();

                auto value = identifier_at(*token_result.value());
                if (value == "proc") {
                    report_parse_error("Procedure must be top-level expression.");
                    recover(); // TODO: Jump to end of procedure and not next ';' or '}'
                    return std::nullopt;
                }

                // Look ahead after the identifier.
                token_result = expect_any();
                if (!token_result)
                    return std::nullopt;

                switch (token_result.value()->type) {
                    default:
                        return parse_reference(context, value);
                    case '(':
                        eat();
                        return parse_call(context, value);
                }
            }
            case TOKEN_INTEGER: {
                eat();
                return parse_number_literal(context, integer_at(*token_result.value()), NUMBER_INTEGER);
            }
            case TOKEN_DECIMAL: {
                eat();
                return parse_number_literal(context, decimal_at(*token_result.value()), NUMBER_DECIMAL);
            }
            case TOKEN_STRING: {
                eat();
                return parse_string_literal(context, string_at(*token_result.value()));
            }
            case '(': {
                eat();
                return parse_parentheses(context);
            }
        }
    }

    std::optional<expression> parser::parse_binary(parse_context context,
        const expression& left, const binary_operator* op)
    {
        auto right_result = parse_expression(context);
        if (!right_result)
            return std::nullopt;

        return expression{
            expression_binary{
                // TODO: Delete this later.    
                new expression{ left },
                new expression{ right_result.value() },
                op
            }
        };
    }
    
    std::optional<expression> parser::parse_parentheses(parse_context context)
    {
        // Parse what is inside the parentheses.
        auto expr_result = parse_expression(CONTEXT_NONE);
        
        // Eat the ')'.
        if (!expect(')')) {
            recover();
            return std::nullopt;
        }

        if(context == CONTEXT_STATEMENT) {
            if (!expect(';')) {
                recover();
                return std::nullopt;
            }
        }

        // If a binary expression is encased in the parentheses,
        // return a "expression_parentheses" containing the binary expression
        // so that operator precedence can be correctly applied later.
        // 
        // Otherwise just return the expression.
        if(expr_result.value().value.empty.type == EXPR_BINARY)
            return expression{ expression_parentheses{ expr_result.value().value.binary.value } };
        else
            return expr_result;
    }
    
    std::optional<expression> parser::parse_number_literal(parse_context context,
        const std::string& number, number_type type)
    {
        if (context == CONTEXT_STATEMENT) {
            if (!expect(';')) {
                recover();
                return std::nullopt;
            }
        }
        
        return expression{ expression_number_literal{ type, number } };
    }
    
    std::optional<expression> parser::parse_string_literal(parse_context context, const std::string& str)
    {
        if (context == CONTEXT_STATEMENT) {
            if (!expect(';')) {
                recover();
                return std::nullopt;
            }
        }
        
        return expression{ expression_string_literal{ str } };
    }

    std::optional<expression> parser::parse_reference(parse_context context, const std::string& identifier)
    {
        auto reference_result = current_scope()->find_symbol(
            identifier, current_package->package_scope);
        
        // Check if symbol is visible in current scope.
        if (!reference_result) {
            report_parse_error("Undefined symbol or symbol is not visible in current scope.");
            return std::nullopt;
        }

        if (context == CONTEXT_STATEMENT) {
            if (!expect(';')) {
                recover();
                return std::nullopt;
            }
        }

        return expression{ expression_reference{ reference_result.value() } };
    }
    
    std::optional<expression> parser::parse_variable_declaration(parse_context context,
        const std::string& variable_name, u8 specifiers)
    {
        bool is_pointer;
        
        auto token_result = expect_any();
        if (!token_result)
            return std::nullopt;
        
        if (token_result.value()->type == '^') {
            is_pointer = true;
            
            token_result = expect_identifier();
            if (!token_result) {
                recover();
                return std::nullopt;
            }
        }
        else {
            is_pointer = false;

            if (token_result.value()->type != TOKEN_IDENTIFIER) {
                report_parse_error("Expected an identifier.");
                recover();
                return std::nullopt;
            }
        }
        
        // The token is an identifier.

        const std::string& type_identifier = identifier_at(*token_result.value());
        
        // Check if the type is visible in current scope.
        std::optional<type> find_type_result = current_scope()->find_type(
            type_identifier, current_package->package_scope);
        
        if (!find_type_result) {
            report_parse_error("Undefined type or type is not visible in current scope.");
            recover();
            return std::nullopt;
        }
        
        // Add variable to symbol table of current scope.
        bool add_symbol_result = current_scope()->add_symbol(
            symbol{ variable_name, SYMBOL_VARIABLE, type_identifier }, current_package->package_scope);

        if (!add_symbol_result) {
            report_parse_error("Symbol is already defined.");
            recover();
            return std::nullopt;
        }

        if (context == CONTEXT_STATEMENT) {
            token_result = expect_any();
            if (!token_result) {
                recover();
                return std::nullopt;
            }
            
            // Variable declaration statements can optionally assign a value.
            if (token_result.value()->type == OP_EQUALS.op_code) {
                // Parse the right-hand side.
                return parse_binary(
                    CONTEXT_STATEMENT,
                    expression{
                        expression_variable_declaration{
                            symbol{ variable_name },
                            find_type_result.value().name,
                            specifiers,
                            is_pointer
                        } 
                    },
                    &OP_EQUALS
                );
            }

            // No value is assigned to the variable, handle end of statement here.
            if (token_result.value()->type != ';') {
                report_parse_error("Expected \";\".");
                recover();
                return std::nullopt;
            }
        }

        return expression{
            expression_variable_declaration{
                symbol{ variable_name },
                find_type_result.value().name,
                specifiers,
                is_pointer
            }
        };
    }

    std::optional<expression> parser::parse_call(parse_context context, const std::string& procedure_name)
    {
        // Check if the procedure is visible in current scope.
        auto proc_result = current_scope()->find_symbol(
            procedure_name, current_package->package_scope);
        
        if (!proc_result) {
            report_parse_error("Undefined symbol or symbol is not visible in current scope.");
            return std::nullopt;
        }
        
        expression_procedure_call call_expr{ symbol{ procedure_name, SYMBOL_PROCEDURE } };

        auto token_result = expect_any();
        if (!token_result)
            return std::nullopt;

        if (token_result.value()->type != ')') {
            // Parse argument(s).
            while (true) {
                auto argument = parse_expression(CONTEXT_NONE);
                if (!argument)
                    return std::nullopt;

                call_expr.argument_list.push_back(argument.value());

                token_result = expect_any();
                if (!token_result)
                    return std::nullopt;

                switch (token_result.value()->type) {
                    default:
                        report_parse_error("Unexpected token.");
                        recover();
                        return std::nullopt;
                    case ',':
                        continue;
                    case ')':
                        break;
                };
            }
        }

        if (context == CONTEXT_STATEMENT) {
            if (!expect(';')) {
                recover();
                return std::nullopt;
            }
        }

        return std::optional<expression>{ expression{ call_expr } };
    }

    std::vector<expression> parser::parse_argument_list()
    {
        std::vector<expression> argument_list;

        // Get the first token after "(".
        // No need to check if it's valid because that was done before in 'parse_procedure()'.
        //std::optional<token*> token_result = eat();

        while (true)
        {
            auto specifiers_result = parse_specifiers();
            if (!specifiers_result)
                return std::vector<expression>{};

            // Next token is guaranteed to exist and be an identifier.
            auto token_result = peek_token();
            const std::string& identifier = identifier_at(*token_result.value());
            eat();

            if (!expect(':')) {
                // TODO: Recover from this by going to the next argument, instead of ";" or "}".
                recover();
                return std::vector<expression>{};
            }

            // Parse the argument
            auto variable_declaration = parse_variable_declaration(CONTEXT_NONE,
                identifier, specifiers_result.value());
            
            if (!variable_declaration)
                return std::vector<expression>{};

            argument_list.push_back(variable_declaration.value());

            token_result = peek_token();
            if (!token_result) {
                report_parse_error("Expected a token.");
                done = true;
                return std::vector<expression>{};
            }

            switch (token_result.value()->type) {
                default:
                    report_parse_error("Unexpected token.");
                    recover();
                    return std::vector<expression>{};
                case ',':
                    eat();
                    continue;
                case ')':
                    eat();
                    return argument_list;
            }
        }
    }

    std::optional<expression> parser::parse_procedure()
    {
        auto token_result = expect_identifier();
        if (!token_result) {
            // TODO: Recover by going to the end of the procedure.
            recover();
            return std::nullopt;
        }

        const std::string& procedure_identifier = identifier_at(*token_result.value());

        // Add procedure to symbol table of current scope
        bool add_symbol_result = current_scope()->add_symbol(
            symbol{ procedure_identifier, SYMBOL_PROCEDURE }, current_package->package_scope
        );
        
        if (!add_symbol_result) {
            report_parse_error("Symbol is already defined.");
            return std::nullopt;
        }

        token_result = expect('(');
        if (!token_result) {
            recover();
            return std::nullopt;
        }

        token_result = peek_token();
        if (!token_result) {
            report_parse_error("Expected a token.");
            done = true;
            return std::nullopt;
        }

        // If a return type is specified, it will be stashed here.
        std::optional<type> return_type_result;

        // Any arguments are stashed here, otherwise it's empty.
        std::vector<expression> argument_list;

        // Procedure has no arguments.
        if (token_result.value()->type == ')') {
            // Eat the ")".
            eat();
        }
        // Procedure has arguments.
        else {
            // Parse argument list.
            argument_list = parse_argument_list();
        }
        
        token_result = peek_token();
        if (!token_result) {
            report_parse_error("Expected a token.");
            done = true;
            return std::nullopt;
        }

        // Return type specified.
        if (token_result.value()->type == TOKEN_RIGHT_POINTER)
        {
            // Eat the "->".
            eat();

            token_result = expect_identifier();
            if (!token_result) {
                // TODO: Recover by jumping to the end of the procedure.
                recover();
                return std::nullopt;
            }

            const std::string& return_type_identifier = identifier_at(*token_result.value());

            // Check if the type is visible in current scope
            return_type_result = current_scope()->find_type(
                return_type_identifier, current_package->package_scope
            );
            
            if (!return_type_result) {
                report_parse_error("Undefined type or type is not visible in current scope.");
                return std::nullopt;
            }

            // Peek the token after the return type.
            token_result = peek_token();
            if (!token_result) {
                report_parse_error("Expected a token");
                done = true;
                return std::nullopt;
            }
        }
        
        switch (token_result.value()->type) {
            default:
                report_parse_error("Unexpected token.");
                recover();
                return std::nullopt;
            // Procedure has a body.
            case '{':
                // Parse procedure body.
                eat();
                return parse_procedure_body(
                    expression_procedure_prototype{
                        symbol { procedure_identifier, SYMBOL_PROCEDURE },
                        argument_list,
                        return_type_result ? return_type_result.value().name : TYPE_VOID.name
                    }
                );

            // Procedure has no body and is a prototype.
            case ';':
                // Done parsing procedure prototype.
                eat();
                return expression{
                    expression_procedure_prototype {
                        symbol { procedure_identifier, SYMBOL_PROCEDURE },
                        argument_list,
                        return_type_result ? return_type_result.value().name : TYPE_VOID.name
                    }
                };
        }
    }

    std::optional<expression> parser::parse_procedure_body(const expression_procedure_prototype& prototype)
    {
        auto token_result = peek_token();
        if (!token_result) {
            report_parse_error("Expected a token.");
            done = true;
            return std::nullopt;
        } 
        
        if (token_result.value()->type == '}') {
            // Procedure body is empty.
            eat();
            return expression{ expression_procedure_definition{ prototype } };
        }

        // Create new scope (only if procedure body is not empty).
        scope_index parent_scope_index = current_scope_index;
        current_scope_index = current_scope()->add_child(scope{ prototype.name.name });

        std::vector<expression> body;

        while (true) {
            auto statement_result = parse_statement();
            if (!statement_result)
                return std::nullopt;

            body.push_back(statement_result.value());

            token_result = peek_token();
            if (!token_result) {
                current_scope_index = parent_scope_index;
                report_parse_error("Expected a token");
                done = true;
                return std::nullopt;
            }

            // Check if the token after the last parsed expression is '}',
            // which would be the end of the body.
            if (token_result.value()->type == '}') {
                // Eat the "}".
                eat();
                break;
            }
        }

        current_scope_index = parent_scope_index;
        return expression{ expression_procedure_definition{ prototype, body } };
    }
    
    std::optional<expression> parser::parse_package_declaration()
    {
        std::string package_name;
        
        while(true)
        {
            auto token_result = expect_identifier();
            if (!token_result) {
                recover();
                return std::nullopt;
            }
            
            package_name += identifier_at(*token_result.value());
            
            token_result = expect_any();
            if (!token_result)
                return std::nullopt;
            
            if(token_result.value()->type == '.') {
                package_name += ".";
                continue;
            }
            else if(token_result.value()->type == ';') {
                // Done parsing package declaration statement.
                set_package(package_name);
                return expression{ expression_package_declaration{ package_name } };
            }
            else {
                report_parse_error("Unexpected token.");
                recover();
                return std::nullopt;
            }
        }
    }
    
    std::optional<expression> parser::parse_package_import()
    {
        std::string package_name;
        
        while(true)
        {
            auto token_result = expect_identifier();
            if (!token_result) {
                recover();
                return std::nullopt;
            }

            package_name += identifier_at(*token_result.value());
            
            token_result = expect_any();
            if (!token_result)
                return std::nullopt;

            // TODO: Check for "as" token.
            if(token_result.value()->type == '.') {
                package_name += ".";
                continue;
            }
            else if(token_result.value()->type == ';') {
                // Done parsing package import statement.
                current_package->imports.insert(package_import{ package_name });
                return expression{ expression_package_import{ package_name } };
            }
            else {
                report_parse_error("Unexpected token.");
                recover();
                return std::nullopt;
            }
        }
    }
    
    expression_binary* get_binary_expression(expression* expr)
    {
        if(expr->value.empty.type == EXPR_BINARY)
            return &expr->value.binary.value;
        else if(expr->value.empty.type == EXPR_PARENTHESES)
            return &expr->value.parentheses.value.expr;
        
        return nullptr;
    }
}
