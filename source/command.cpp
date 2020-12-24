#include <command.hpp>

#include <log.hpp>
#include <io.hpp>
#include <build.hpp>

#include <iostream>
#include <cstdlib>

namespace masonc
{
    void execute_command_help(const command_parsed& command)
    {
        // TODO: Create and sort pointers to key-value pairs in `COMMANDS`
        //       by `command_definition::order` to get them printed in the same order
        //       in which they are defined.

        std::string output;

        for(auto it = COMMANDS.begin(); it != COMMANDS.end(); it++) {
            std::string command_usage = "Command: " + std::string{ it->first };
            std::string command_description = "Description: " + std::string{ it->second.description } + "\n";
            std::string command_argument_list;

            for(u64 i = 0; i < it->second.arguments.size(); i += 1) {
                const command_argument_definition& argument = it->second.arguments[i];
                const char* argument_type_name = command_argument_type_string(argument.type);

                command_usage += " <" + std::string{ argument.name } + ">";
                command_argument_list += "    Argument: <" + std::string{ argument.name } + ">\n" +
                                         "    Type: " + argument_type_name + "\n" +
                                         "    Description: " + argument.description + "\n\n";
            }

            for(auto option : it->second.options) {
                const char* option_name = option.first;
                const char* option_type_name = command_argument_type_string(option.second.type);
                const char* option_description = option.second.description;

                command_usage += " -" + std::string{ option_name } + "=<" + option_type_name + ">";
                command_argument_list += "    Option: --" + std::string{ option_name } + "\n" +
                                       //"    Type: " + option_type_name + "\n" +
                                         "    Description: " + option_description + "\n\n";
            }

            command_usage += "\n";
            command_description += "\n";
            command_argument_list += "\n";

            output += command_usage + command_description + command_argument_list;
        }

        std::cout << "\n" << output << std::flush;
    }

    void execute_command_usage(const command_parsed& command)
    {
        const char* key = command.parsed_arguments[0].second.str;
        auto find_command_it = COMMANDS.find(key);

        if (find_command_it == COMMANDS.end()) {
            std::cout << "Command \"" << key << "\" does not exist." << std::endl;
            return;
        }

        const command_definition& definition = find_command_it->second;
        const char* command_usage = find_command_it->first;

        // TODO: Implement the "usage" command.
    }

    void execute_command_exit(const command_parsed& command)
    {
        std::exit(0);
    }

    void execute_command_build(const command_parsed& command)
    {
        // TODO: Handle "add_extensions" option.
        const char* sources = command.parsed_arguments[0].second.str;
        const char* object_file_name = command.parsed_arguments[1].second.str;

        std::vector<path> split_sources;
        std::string temp;

        const char* current_char = sources;
        while (true) {
            if (*current_char == '\n') {
                split_sources.push_back(path{ temp });
                temp = "";
                current_char += 1;
            }
            else if (*current_char == '\0') {
                split_sources.push_back(path{ temp });
                break;
            }
            else {
                temp += *current_char;
                current_char += 1;
            }
        }

        build_object(split_sources);
    }

    bool execute_command(const std::string& input)
    {
        std::cout << input << std::endl;

        masonc::lexer::lexer_instance command_lexer;
        masonc::lexer::lexer_instance_output output;

        std::optional<command_parsed> command_result = parse_command(&command_lexer, &output,
            input.c_str(), static_cast<u64>(input.length()));

        if(!command_result)
            return false;

        const command_parsed& command = command_result.value();
        command.definition->executor(command);

        return true;
    }

    bool listen_command(masonc::lexer::lexer_instance* command_lexer)
    {
        std::string input;
        masonc::lexer::lexer_instance_output output;

        std::getline(std::cin, input);

        std::optional<command_parsed> command_result = parse_command(command_lexer, &output,
            input.c_str(), static_cast<u64>(input.length()));

        if(!command_result)
            return false;

        const command_parsed& command = command_result.value();
        command.definition->executor(command);

        return true;
    }

    const char* command_argument_type_string(command_argument_type argument_type)
    {
        switch(argument_type) {
            default:
                log_error("Not implemented command argument type.");
                return "";
            case command_argument_type::INTEGER:
                return "Integer";
            case command_argument_type::DECIMAL:
                return "Decimal";
            case command_argument_type::STRING:
                return "String";
        }
    }

    std::optional<command_name_pair*> find_command(masonc::lexer::lexer_instance_output* output)
    {
        if(output->tokens.size() == 0) {
            // User entered nothing or whitespace only.
            std::cout << "Expected command name." << std::endl;
            return std::nullopt;
        }
        masonc::lexer::token *command_token = &output->tokens[0];

        if(command_token->type != masonc::lexer::TOKEN_IDENTIFIER) {
            // User did not enter a valid command name.
            std::cout << "Expected command name." << std::endl;
            return std::nullopt;
        }

        const char* command_name = output->identifiers.at(command_token->value_index);

        auto find_command_it = COMMANDS.find(command_name);
        if(find_command_it == COMMANDS.end()) {
            // User entered a command that does not exist.
            std::cout << "Command \"" << command_name << "\" does not exist." << std::endl;
            return std::nullopt;
        }

        return std::make_optional(&(*find_command_it));
    }

    std::optional<command_parsed> parse_command(masonc::lexer::lexer_instance* command_lexer,
        masonc::lexer::lexer_instance_output* output, const char* input, u64 input_size)
    {
        // Tokenize the input.
        command_lexer->tokenize(input, input_size, output);
        if(output->messages.errors.size() != 0) {
            std::cout << "Failed to tokenize command." << std::endl;
            return std::nullopt;
        }

        // Find the command.
        auto find_result = find_command(output);
        if(!find_result) {
            return std::nullopt;
        }

        command_parsed command;
        command.name = find_result.value()->first;
        command.definition = &find_result.value()->second;

        u64 token_index = 1;

        // Parse arguments.
        for(u64 i = 0; i < command.definition->arguments.size(); i += 1) {
            if(token_index >= output->tokens.size()) {
                std::cout << "Missing argument(s)." << std::endl;
                return std::nullopt;
            }

            auto argument_result = parse_command_argument(&token_index,
                output, command.definition->arguments[i].type);

            if(!argument_result)
                return std::nullopt;

            command.parsed_arguments.push_back(argument_result.value());
        }

        // Parse optional arguments.
        while(token_index < output->tokens.size()) {
            auto option_result = parse_command_option(&token_index,
                output, command.definition->options);

            if(!option_result)
                return std::nullopt;

            command.parsed_options.push_back(option_result.value());
        }

        return std::make_optional(command);
    }

    std::optional<command_argument_pair> parse_command_argument(u64* token_index,
        masonc::lexer::lexer_instance_output* output, command_argument_type argument_type)
    {
        command_argument_value value;

        masonc::lexer::token* current_token = &output->tokens[*token_index];
        *token_index += 1;

        switch(argument_type) {
            default:
                // Not implemented argument type.
                return std::nullopt;

            case command_argument_type::INTEGER:
                if(current_token->type != masonc::lexer::TOKEN_INTEGER) {
                    std::cout << "Mismatched type, expected an integer." << std::endl;
                    return std::nullopt;
                }

                value.integer = static_cast<s64>(std::stoll(output->integers.at(current_token->value_index)));
                return std::make_optional(command_argument_pair{ command_argument_type::INTEGER, value });

            case command_argument_type::DECIMAL:
                if(current_token->type != masonc::lexer::TOKEN_DECIMAL) {
                    std::cout << "Mismatched type, expected a decimal." << std::endl;
                    return std::nullopt;
                }

                value.decimal = static_cast<f64>(std::stod(output->decimals.at(current_token->value_index)));
                return std::make_optional(command_argument_pair{ command_argument_type::DECIMAL, value });

            case command_argument_type::STRING:
                if(current_token->type != masonc::lexer::TOKEN_STRING) {
                    std::cout << "Mismatched type, expected a string." << std::endl;
                    return std::nullopt;
                }

                value.str = output->strings.at(current_token->value_index);
                return std::make_optional(command_argument_pair{ command_argument_type::STRING, value });
        }
    }

    std::optional<command_option_tuple> parse_command_option(u64* token_index,
        masonc::lexer::lexer_instance_output* output, const std::map<const char*,
        command_option_definition>& options)
    {
        if(*token_index + 4 >= output->tokens.size()) {
            std::cout << "Incomplete option." << std::endl;
            return std::nullopt;
        }

        // Relevant tokens of the option "-identifier=".
        masonc::lexer::token* dash_token = &output->tokens[*token_index];
        masonc::lexer::token* dash_token_2 = &output->tokens[*token_index + 1];
        masonc::lexer::token* identifier_token = &output->tokens[*token_index + 2];
        masonc::lexer::token* equals_token = &output->tokens[*token_index + 3];
        *token_index += 4;

        if(dash_token->type != '-') {
            std::cout << "Invalid option format, expected \"--\"." << std::endl;
            return std::nullopt;
        }
        if(dash_token_2->type != '-') {
            std::cout << "Invalid option format, expected \"--\"." << std::endl;
            return std::nullopt;
        }
        if(identifier_token->type != masonc::lexer::TOKEN_IDENTIFIER) {
            std::cout << "Invalid option format, expected identifier." << std::endl;
            return std::nullopt;
        }
        if(equals_token->type != '=') {
            std::cout << "Invalid option format, expected \"=\"." << std::endl;
            return std::nullopt;
        }

        const char* option_name = output->identifiers.at(identifier_token->value_index);
        auto option_find_it = options.find(option_name);

        if(option_find_it == options.end()) {
            std::cout << "Option not found for this command." << std::endl;
            return std::nullopt;
        }

        const command_argument_type& option_type = option_find_it->second.type;

        // Parse the value
        auto value_result = parse_command_argument(token_index, output, option_type);
        if(!value_result)
            return std::nullopt;

        *token_index += 1;
        return std::make_optional(
            command_option_tuple{
                value_result.value().first, value_result.value().second, option_name
            }
        );
    }
}