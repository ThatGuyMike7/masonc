#include <lexer.hpp>
#include <timer.hpp>

#include <build.hpp>

#include <cctype>
#include <string>
#include <iostream>
#include <optional>

namespace masonc::lexer
{
    bool is_escape_sequence(char c)
    {
        return (c == 'n' || c == 't' || c == '0' || c == '\\');
    }

    char get_escape_char(char sequence)
    {
        switch(sequence) {
            default:
                return '\0';
            case 'n':
                return '\n';
            case 't':
                return '\t';
            case '0':
                return '\0';
            case '\\':
                return '\\';
            case '"':
                return '"';
            case '\'':
                return '\'';
        }
    }

    bool is_space(char c)
    {
        return (c == ' ' || c == '\n' || c == '\t');
    }

    std::string get_composed_token(token_type type)
    {
        for(u64 i = 0; i < COMPOSED_TOKENS_LENGTH / 2; i += 1) {
            if(COMPOSED_TOKEN_TYPES[i] == type) {
                std::string str;
                str.push_back(COMPOSED_TOKENS[i * 2]);
                str.push_back(COMPOSED_TOKENS[i * 2 + 1]);
                return str;
            }
        }

        return "";
    }

    std::optional<char> lexer_instance::get_char()
    {
        char c = this->input[this->char_index];
        if(c == '\0')
            return std::optional<char>{};

        if (c == '\n') {
            line_number += 1;
            this->column_number = 1;
        }
        else if(c == '\t') {
            this->column_number += this->tab_size;
        }
        else {
            this->column_number += 1;
        }

        this->char_index += 1;
        return std::optional<char>{ c };
    }

    std::optional<char> lexer_instance::peek_char()
    {
        char c = this->input[this->char_index];
        if(c == '\0')
            return std::optional<char>{};

        return std::optional<char>{ c };
    }

    void lexer_instance::add_identifier(const std::string& identifier, u64 start_column, u64 end_column)
    {
        token tok = { this->output->identifiers.size(), TOKEN_IDENTIFIER };
        token_location location = { this->line_number, start_column, end_column };

        this->output->tokens.push_back(tok);
        this->output->locations.push_back(location);
        this->output->identifiers.copy_back(identifier);
    }

    void lexer_instance::add_number(const std::string& number, bool dot, u64 start_column, u64 end_column)
    {
        if (dot) {
            if (number.back() == '.') {
                // Error
                token_location error_location{ this->line_number, this->char_index, this->char_index };
                output->messages.report_error(
                    "Expected digit after \".\" in decimal.",
                    build_stage::LEXER,
                    error_location
                );

                return;
            }

            token tok = { this->output->decimals.size(), TOKEN_DECIMAL };
            this->output->tokens.push_back(tok);
            this->output->decimals.copy_back(number);
        }
        else {
            token tok = { this->output->integers.size(), TOKEN_INTEGER };
            this->output->tokens.push_back(tok);
            this->output->integers.copy_back(number);
        }

        token_location location = { this->line_number, start_column, end_column };
        this->output->locations.push_back(location);
    }

    void lexer_instance::tokenize(const char* input, u64 input_size, lexer_instance_output* output, u8 tab_size)
    {
        // Reset the lexer.
        prepare(input, input_size, output, tab_size);

        // Do the actual work.
        return analyze();
    }

    void lexer_instance::prepare(const char* input, u64 input_size, lexer_instance_output* output, u8 tab_size)
    {
        this->input = input;
        this->output = output;
        this->tab_size = tab_size;

        this->char_index = 0;
        this->line_number = 1;
        this->column_number = 1;

        try {
            // Guess how many characters will end up being 1 token on average to avoid reallocations.
            const u64 characters_per_token_guess = input_size / 3 + 32;

            output->tokens.reserve(characters_per_token_guess);
            output->locations.reserve(characters_per_token_guess);
        }
        catch (...) {
            log_warning("Could not reserve space for token vector");
        }
    }

    void lexer_instance::analyze()
    {
        // Get the first character.
        std::optional<char> char_result = get_char();
        if (!char_result)
            return;

        while (true) {
            // Skip whitespace / newline etc.
            while (is_space(char_result.value())) {
                char_result = get_char();
                if (!char_result)
                    return;
            }

            // String
            if(char_result.value() == '"') {
                u64 string_start_column = this->column_number - 1;
                std::string str;
                char prev_char = char_result.value();

                while(true) {
                    char_result = get_char();
                    if(!char_result) {
                        // Error
                        token_location error_location{ this->line_number, string_start_column,
                            this->column_number - 1 };

                        output->messages.report_error("Expected '\"'", build_stage::LEXER,
                            error_location);

                        return;
                    }

                    // Lex escape sequence.
                    if(char_result.value() == ESCAPE_BEGIN) {
                        u64 escape_start_column = this->column_number - 1;

                        char_result = get_char();
                        if(!char_result) {
                            // Error
                            token_location error_location{ this->line_number, string_start_column,
                                this->column_number - 1 };

                            output->messages.report_error("Expected '\"'", build_stage::LEXER,
                                error_location);

                            return;
                        }

                        // TODO: Continue work here

                        if(!is_escape_sequence(char_result.value()) && char_result.value() != '"') {
                            // Error: Not valid escape sequence.
                            token_location error_location{ this->line_number, escape_start_column,
                                this->column_number - 1 };

                            output->messages.report_error(
                                "'\\"
                                + std::string{ char_result.value() }
                                + "' is not a valid escape sequence",
                                build_stage::LEXER,
                                error_location
                            );

                            // Skip until end of string.
                            do {
                                char_result = get_char();
                                if(!char_result) {
                                    // Error
                                    error_location = token_location{ this->line_number, string_start_column,
                                        this->column_number - 1};

                                    output->messages.report_error(
                                        "Expected '\"'",
                                        build_stage::LEXER,
                                        error_location
                                    );

                                    return;
                                }

                            } while(char_result.value() != '"');

                            // Successfully skipped string, lex the next token.
                            continue;
                        }

                        char escape_char = get_escape_char(char_result.value());
                        str.push_back(escape_char);
                        prev_char = escape_char;
                        continue;
                    }

                    if(char_result.value() == '"')
                        break;

                    str.push_back(char_result.value());
                    prev_char = char_result.value();
                }

                token tok = { this->output->strings.size(), TOKEN_STRING };

                token_location location = {
                    this->line_number,
                    string_start_column,
                    this->column_number - 1
                };

                this->output->tokens.push_back(tok);
                this->output->strings.copy_back(str);
                this->output->locations.push_back(location);

                char_result = get_char();
                if(!char_result)
                    return;

                // Done lexing constant string literal
                continue;
            }

            // Identifier starting with an alpha character (a-z, A-Z) or underscore and
            // further characters being alpha, underscores or numeric (0-9).
            if (is_alpha(char_result.value())) //|| char_result.value() == '_')
            {
                u64 identifier_start_column = this->column_number - 1;
                std::string identifier;

                // Read the identifier.
                do {
                    identifier.push_back(char_result.value());

                    char_result = get_char();
                    if (!char_result) {
                        // Add identifier as it is.
                        add_identifier(identifier, identifier_start_column, this->column_number - 1);
                        return;
                    }

                } while (is_alnum(char_result.value())); //|| char_result.value() == '_');

                // TODO: Potentially check if it's a language-defined identifier.

                // Done lexing identifier.
                add_identifier(identifier, identifier_start_column, this->column_number - 1);
                continue;
            }

            // Number (integer or decimal)
            if (is_num(char_result.value())) {
                u64 number_start_column = this->column_number - 1;
                std::string number;

                // There can only be one "." in a decimal.
                bool dot = false;

                // Read the number.
                do {
                    number.push_back(char_result.value());

                    char_result = get_char();
                    if (!char_result) {
                        // Add number as it is
                        add_number(number, dot, number_start_column, this->column_number - 1);
                        return;
                    }

                    if (char_result.value() == '.') {
                        // A second dot in this number?
                        if (dot) {
                            // Error
                            token_location error_location{ this->line_number, number_start_column,
                                this->column_number - 1 };

                            output->messages.report_error(
                                "Decimal contains two '.'",
                                build_stage::LEXER,
                                error_location
                            );

                            return;
                        }

                        dot = true;
                        number.push_back(char_result.value());

                        char_result = get_char();
                        if (!char_result) {
                            // Add number as it is.
                            add_number(number, dot, number_start_column, this->column_number - 1);
                            return;
                        }
                    }

                } while (is_num(char_result.value()));

                // Done lexing number.
                add_number(number, dot, number_start_column, this->column_number - 1);
                continue;
            }

            // Comments
            if (char_result.value() == '/') {
                std::optional<char> peek_token_result = peek_char();
                if (peek_token_result) {
                    // Comment until end of line.
                    if (peek_token_result.value() == '/') {
                        // Eat the peeked token.
                        get_char();

                        do {
                            char_result = get_char();
                            if (!char_result)
                                return;

                        } while (char_result.value() != '\n');

                        // Eat the "\n".
                        char_result = get_char();
                        if (!char_result)
                            return;

                        // Done lexing line comment.
                        continue;
                    }

                    // Block comment
                    if (peek_token_result.value() == '*') {
                        // Eat the peeked token.
                        get_char();

                        u64 nests = 1;

                        do {
                            char_result = get_char();
                            if (!char_result)
                                return;

                            if (char_result.value() == '/') {
                                peek_token_result = peek_char();
                                if (!peek_token_result)
                                    return;

                                // Nested block comment.
                                if (peek_token_result.value() == '*') {
                                    nests += 1;
                                }

                                // Eat the peeked token.
                                get_char();
                            }
                            else if (char_result.value() == '*') {
                                peek_token_result = peek_char();
                                if (!peek_token_result)
                                    return;

                                // End of a block comment.
                                if (peek_token_result.value() == '/') {
                                    nests -= 1;
                                }

                                // Eat the peeked token.
                                get_char();
                            }

                        } while (nests > 0);

                        char_result = get_char();
                        if (!char_result)
                            return;

                        // Done lexing block comment(s).
                        continue;
                    }
                }
            }

            // Composed tokens
            u64 i = 0;
            bool is_composed = false;
            while (i < COMPOSED_TOKENS_LENGTH) {
                if (char_result.value() == COMPOSED_TOKENS[i]) {
                    // It could be a composed token.

                    std::optional<char> peek_token_result = peek_char();
                    if (!peek_token_result) {
                        // Not a composed token, create an ASCII token.
                        token tok;
                        tok.type = char_result.value();

                        token_location location = {
                            this->line_number,
                            this->column_number - 1,	// current token column
                            this->column_number - 1
                        };

                        this->output->tokens.push_back(tok);
                        this->output->locations.push_back(location);

                        return;
                    }

                    if (peek_token_result.value() == COMPOSED_TOKENS[i + 1]) {
                        // It is a composed token.
                        token tok;
                        tok.type = COMPOSED_TOKEN_TYPES[i / 2];

                        token_location location = {
                            this->line_number,
                            this->column_number - 1,	// current token column
                            this->column_number			// peeked token column
                        };

                        this->output->tokens.push_back(tok);
                        this->output->locations.push_back(location);

                        // Eat the current token and the peeked token.
                        get_char();
                        char_result = get_char();
                        if (!char_result)
                            return;

                        is_composed = true;
                        break;
                    }
                }

                i += 2;
            }

            // Done lexing composed token.
            if (is_composed)
                continue;

            // Otherwise just create an ASCII token.
            token tok;
            tok.type = char_result.value();

            token_location location = {
                this->line_number,
                this->column_number - 1,	// current token column
                this->column_number - 1
            };

            this->output->tokens.push_back(tok);
            this->output->locations.push_back(location);

            char_result = get_char();
            if (!char_result)
                return;
        }
    }

    void lexer_instance::print_tokens()
    {
        for(u64 i = 0; i < output->tokens.size(); i += 1) {
            token* current_token = &this->output->tokens[i];
            token_location* location = &this->output->locations[i];

            if(current_token->type < 0) {
                switch(current_token->type) {
                    default:
                        std::cout << "Unknown Token" << std::endl;
                        break;
                    case TOKEN_IDENTIFIER:
                        std::cout << "Identifier Token (l. " << location->line_number << "): " <<
                                      this->output->identifiers.at(current_token->value_index) <<
                                      std::endl;
                        break;
                    case TOKEN_INTEGER:
                        std::cout << "Integer Token (l. " << location->line_number << "): " <<
                                      this->output->integers.at(current_token->value_index) <<
                                      std::endl;
                        break;
                    case TOKEN_DECIMAL:
                        std::cout << "Decimal Token (l. " << location->line_number << "): " <<
                                      this->output->decimals.at(current_token->value_index) <<
                                      std::endl;
                        break;
                    case TOKEN_STRING:
                        std::cout << "String Token (l. " << location->line_number << "): " <<
                                      this->output->strings.at(current_token->value_index) <<
                                      std::endl;
                        break;
                    case TOKEN_DOUBLECOLON:
                    case TOKEN_RIGHT_POINTER:
                        std::cout << "Composed Token (l. " << location->line_number << "): " <<
                                      get_composed_token(
                                          static_cast<token_type>(current_token->type)
                                      ) <<
                                      std::endl;
                        break;
                }
            }
            else {
                std::cout << "ASCII Token (l. " << location->line_number << "): " <<
                              static_cast<char>(current_token->type) << std::endl;
            }
        }
    }
}