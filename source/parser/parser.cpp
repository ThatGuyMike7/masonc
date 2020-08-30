#include <parser.hpp>

#include <common.hpp>
#include <type.hpp>
#include <log.hpp>
#include <timer.hpp>
#include <lexer.hpp>

#include <iostream>
#include <optional>
#include <limits>
#include <cstring>

namespace masonc
{
    void parser::parse(lexer_output* input, parser_output* output)
    {
        this->input = input;
        this->output = output;

        this->current_package = nullptr;
        this->current_ast = nullptr;
        this->current_package_handle = 0;

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
                message += "Number Literal: Value='";
                message += expr.value.number.value.value;
                message += "'";
                break;

            case EXPR_STRING_LITERAL:
                message += "String Literal: Value='";
                message += expr.value.str.value.value;
                message += "'";
                break;

            case EXPR_REFERENCE:
                message += "Reference: Name='";
                //message += expr.value.reference.value.name;
                message += "'";
                break;

            case EXPR_VAR_DECLARATION:
                message += "Variable Declaration: Name='";
                //message += expr.value.variable_declaration.value.name;
                message += "' Type='";

                if(expr.value.variable_declaration.value.is_pointer)
                    message += "^";

                //message += expr.value.variable_declaration.value.type_name;
                message += "'";

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
                message += "Procedure Prototype Expression: Name='";
                //message += expr.value.procedure_prototype.value.name;
                message += "' Return Type='";
                //message += expr.value.procedure_prototype.value.return_type_name;
                message += "'";

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
                message += "Procedure Call Expression: Name='";
                //message += expr.value.procedure_call.value.name;
                message += "'";

                for (u64 i = 0; i < expr.value.procedure_call.value.argument_list.size(); i += 1) {
                    message += "\n" + format_expression(
                        expr.value.procedure_call.value.argument_list[i], level + 1
                    );
                }
                break;

            case EXPR_PACKAGE_DECLARATION:
                message += "Package Declaration: Name='";
                message += expr.value.package_declaration.value.package_name;
                message += "'";
                break;

            case EXPR_PACKAGE_IMPORT:
                message += "Package Import: Name='";
                message += expr.value.package_import.value.package_name;
                message += "'";
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

            // Reached the end of the token stream.
            if (done)
                break;
        }
    }

    scope* parser::current_scope()
    {
        return current_package->package_scope.get_child(current_scope_index);
    }

    void parser::set_package(const char* package_name, u16 package_name_length)
    {
        auto search_package_name = output->package_names.find(package_name);

        if (search_package_name) {
            current_package_handle = search_package_name.value();

            // If we found the package name, we can assume the AST and package structure exist too,
            // hence no need to check if they could be found or not.
            auto search_package = output->packages.find(current_package_handle);
            auto search_ast = output->asts.find(current_package_handle);

            current_package = &search_package->second;
            current_ast = &search_ast->second;
        }
        else {
            // Create new package name.
            current_package_handle = output->package_names.copy_back(package_name, package_name_length);

            // Create new package and AST.
            auto inserted_package = output->packages.insert({ current_package_handle, package{} });
            auto inserted_ast = output->asts.insert({ current_package_handle, std::vector<expression>{} });

            assume(inserted_package.second && inserted_ast.second,
                "either package exists and AST does not or AST exists and package does not");

            current_package = &inserted_package.first->second;
            current_ast = &inserted_ast.first->second;

            // Tell the package scope of the package.
            current_package->package_scope.package = current_package;

            u16 package_name_length = output->package_names.length_at(current_package_handle);
            const char* package_name = output->package_names.at(current_package_handle);

            // Give the package scope the package name.
            current_package->package_scope.set_name(package_name, package_name_length);

            try {
                // Guess how many tokens will end up being 1 expression on average to
                // avoid reallocations.
                current_ast->reserve(input->tokens.size() / 10 + 32);
            }
            catch (...) {
                log_warning("Could not reserve space for AST container.");
            }
        }

        current_scope_index = current_package->package_scope.index;
    }

    void parser::set_package(const std::string& package_name)
    {
        assume(package_name.length() <= std::numeric_limits<u16>::max(),
            "\"package_name\" length exceeds size of \"u16\"");

        set_package(package_name.c_str(), package_name.length());
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

        return std::nullopt;
    }

    std::optional<token*> parser::expect_any()
    {
        auto token_result = peek_token();
        if (!token_result) {
            report_parse_error("Expected a token.");
            done = true;
            return std::nullopt;
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
            return std::nullopt;
        }

        if (token_result.value()->type != TOKEN_IDENTIFIER) {
            report_parse_error("Expected an identifier.");
            return std::nullopt;
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
            return std::nullopt;
        }

        // Assumes that type "char" is signed
        if (token_result.value()->type != c) {
            report_parse_error("Expected \"" + std::string{ c } + "\".");
            return std::nullopt;
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
            return std::nullopt;
        }

        if (token_result.value()->type != TOKEN_IDENTIFIER ||
            identifier_at(*token_result.value()) != identifier)
        {
            report_parse_error("Expected \"" + identifier + "\".");
            return std::nullopt;
        }

        eat();
        return token_result.value();
    }

    const char* parser::identifier_at(const token& identifier_token)
    {
        assume(input->identifiers.size() > identifier_token.value_index, "value_index is out of range");
        return input->identifiers.at(identifier_token.value_index);
    }

    const char* parser::integer_at(const token& integer_token)
    {
        assume(input->integers.size() > integer_token.value_index, "value_index is out of range");
        return input->integers.at(integer_token.value_index);
    }

    const char* parser::decimal_at(const token& decimal_token)
    {
        assume(input->decimals.size() > decimal_token.value_index, "value_index is out of range");
        return input->decimals.at(decimal_token.value_index);
    }

    const char* parser::string_at(const token& string_token)
    {
        assume(input->strings.size() > string_token.value_index, "value_index is out of range");
        return input->strings.at(string_token.value_index);
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

            const char* identifier = identifier_at(*token_result.value());

            if (std::strcmp(identifier, "mut") == 0) {
                eat();

                if (specifiers & SPECIFIER_MUT) {
                    report_parse_error("Specifier is already defined.");
                    continue;
                }

                specifiers |= SPECIFIER_MUT;
            }
            else if (std::strcmp(identifier, "const") == 0) {
                eat();

                if (specifiers & SPECIFIER_CONST) {
                    report_parse_error("Specifier is already defined.");
                    continue;
                }

                specifiers |= SPECIFIER_CONST;
            }
            else {
                return specifiers;
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

        const char* identifier = identifier_at(*token_result.value());
        symbol_handle identifier_handle = token_result.value()->value_index;

        eat();

        if (specifiers_result.value() == SPECIFIER_NONE) {
            if (std::strcmp(identifier, "proc") == 0)
                return parse_procedure();
            if (std::strcmp(identifier, "package") == 0)
                return parse_package_declaration();
            if (std::strcmp(identifier, "import") == 0)
                return parse_package_import();
        }

        // Specifiers are declared and parsed.

        token_result = expect(':');
        if (!token_result) {
            recover();
            return std::nullopt;
        }

        return parse_variable_declaration(CONTEXT_STATEMENT,
            identifier_handle, specifiers_result.value());
    }

    std::optional<expression> parser::parse_statement()
    {
        std::optional<u8> specifiers_result = parse_specifiers();
        if(!specifiers_result)
            return std::nullopt;

        // Next token is guaranteed to exist and be an identifier.
        auto token_result = peek_token();

        const char* identifier = identifier_at(*token_result.value());
        symbol_handle identifier_handle = token_result.value()->value_index;

        eat();

        if (std::strcmp(identifier, "return") == 0) {
            // Parse return statement.
            return parse_expression(CONTEXT_STATEMENT);
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
            return parse_call(CONTEXT_STATEMENT, identifier_handle);
        }

        // Specifiers are declared or next token is not "(".

        if (token_result.value()->type == ':') {
            eat();
            return parse_variable_declaration(CONTEXT_STATEMENT,
                identifier_handle, specifiers_result.value());
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
        if (op_result) {
            // Eat the binary operator.
            eat();
        }
        else {
            // Next token is not a binary operator.
            if (context == CONTEXT_STATEMENT) {
                if (token_result.value()->type == ';') {
                    // Eat the ";".
                    eat();
                }
                else {
                    recover();
                    return std::nullopt;
                }
            }

            return primary_result;
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

                expression* expr = new expression{ primary_result.value() };
                delete_list_expressions.push_back(expr);

                return expression{ expression_unary{ expr, token_result.value()->type } };
            }
            case TOKEN_IDENTIFIER: {
                eat();

                const char* identifier = identifier_at(*token_result.value());
                symbol_handle identifier_handle = token_result.value()->value_index;

                if (std::strcmp(identifier, "proc") == 0) {
                    report_parse_error("Procedure must be top-level expression.");
                    // TODO: Jump to end of procedure and not next ";" or "}".
                    recover();
                    return std::nullopt;
                }

                // Look ahead after the identifier.
                token_result = peek_token();
                if (!token_result) {
                    report_parse_error("Expected a token.");
                    done = true;
                    return std::nullopt;
                }

                switch (token_result.value()->type) {
                    default:
                        return parse_reference(context, identifier_handle);
                    case '(':
                        eat();
                        return parse_call(context, identifier_handle);
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

        expression* expr_left = new expression{ left };
        expression* expr_right = new expression{ right_result.value() };

        delete_list_expressions.push_back(expr_left);
        delete_list_expressions.push_back(expr_right);

        return expression{ expression_binary{ expr_left, expr_right, op } };
    }

    std::optional<expression> parser::parse_parentheses(parse_context context)
    {
        // Parse what is inside the parentheses.
        auto expr_result = parse_expression(CONTEXT_NONE);
        if (!expr_result)
            return std::nullopt;

        // Eat the ")".
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
        const char* number, number_type type)
    {
        if (context == CONTEXT_STATEMENT) {
            if (!expect(';')) {
                recover();
                return std::nullopt;
            }
        }

        return expression{ expression_number_literal{ number, type } };
    }

    std::optional<expression> parser::parse_string_literal(parse_context context, const char* str)
    {
        if (context == CONTEXT_STATEMENT) {
            if (!expect(';')) {
                recover();
                return std::nullopt;
            }
        }

        return expression{ expression_string_literal{ str } };
    }

    std::optional<expression> parser::parse_reference(parse_context context,
        symbol_handle identifier_handle)
    {
        if (context == CONTEXT_STATEMENT) {
            if (!expect(';')) {
                recover();
                return std::nullopt;
            }
        }

        return expression{ expression_reference{ identifier_handle } };
    }

    std::optional<expression> parser::parse_variable_declaration(parse_context context,
        symbol_handle name_handle, u8 specifiers)
    {
        const char* name = input->identifiers.at(name_handle);
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

        const char* type = identifier_at(*token_result.value());
        type_handle type_handle = token_result.value()->value_index;

        // Add variable to symbol table of current scope.
        bool add_symbol_result = current_scope()->add_symbol(name);
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
                            name_handle,
                            type_handle,
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
                name_handle,
                type_handle,
                specifiers,
                is_pointer
            }
        };
    }

    std::optional<expression> parser::parse_call(parse_context context, symbol_handle name_handle)
    {
        expression_procedure_call call_expr{ name_handle };

        auto token_result = peek_token();
        if (!token_result) {
            report_parse_error("Expected a token.");
            done = true;
            return std::nullopt;
        }

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
                        goto END_LOOP;
                };
            }
        }

        END_LOOP:
        if (context == CONTEXT_STATEMENT) {
            if (!expect(';')) {
                recover();
                return std::nullopt;
            }
        }

        return expression{ call_expr };
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

            //const char* identifier = identifier_at(*token_result.value());
            symbol_handle identifier_handle = token_result.value()->value_index;

            eat();

            if (!expect(':')) {
                // TODO: Recover from this by going to the next argument, instead of ";" or "}".
                recover();
                return std::vector<expression>{};
            }

            // Parse the argument
            auto variable_declaration = parse_variable_declaration(CONTEXT_NONE,
                identifier_handle, specifiers_result.value());

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

        const char* name = identifier_at(*token_result.value());
        symbol_handle name_handle = token_result.value()->value_index;

        // Add procedure to symbol table of current scope.
        bool add_symbol_result = current_scope()->add_symbol(name);
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

        // Contains return type handle if one is specified, otherwise empty.
        std::optional<type_handle> return_type_handle;

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

            //return_type_identifier = identifier_at(*token_result.value());
            return_type_handle = token_result.value()->value_index;

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
                        name_handle,
                        return_type_handle,
                        argument_list
                    }
                );

            // Procedure has no body and is a prototype.
            case ';':
                // Done parsing procedure prototype.
                eat();
                return expression{
                    expression_procedure_prototype {
                        name_handle,
                        return_type_handle,
                        argument_list
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

        // Procedure body is not empty.

        scope_index parent_scope_index = current_scope_index;

        u16 procedure_name_length = input->identifiers.length_at(prototype.name_handle);
        const char* procedure_name = input->identifiers.at(prototype.name_handle);

        // Create new scope for the procedure and make it current.
        current_scope_index = current_scope()->add_child(scope{});

        scope* procedure_scope = current_scope();
        procedure_scope->set_name(procedure_name, procedure_name_length);

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
        std::string temp_package_name;

        while(true)
        {
            auto token_result = expect_identifier();
            if (!token_result) {
                recover();
                return std::nullopt;
            }

            temp_package_name += identifier_at(*token_result.value());

            token_result = expect_any();
            if (!token_result)
                return std::nullopt;

            if(token_result.value()->type == '.') {
                temp_package_name += ".";
                continue;
            }
            else if(token_result.value()->type == ';') {
                set_package(temp_package_name);
                const char* package_name = output->package_names.at(current_package_handle);

                // Done parsing package declaration statement.
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
                // TODO: Check if imported twice.

                u64 import_handle = current_package->imports.copy_back(package_name);
                package_import import = current_package->imports.at(import_handle);

                // Done parsing package import statement.
                return expression{ expression_package_import{ import } };
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
