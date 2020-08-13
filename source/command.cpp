#include <command.hpp>

#include <log.hpp>
#include <io.hpp>
#include <build.hpp>

#include <iostream>
#include <cstdlib>
#include <string>

namespace masonc
{
    void execute_command_help(const command_parsed& command)
    {
        // TODO: Create and sort pointers to key-value pairs in `COMMANDS`
        //       by `command_definition::order` to get them printed in the same order
        //       in which they are defined.

        std::string output;

        for(auto it = COMMANDS.begin(); it != COMMANDS.end(); it++) {
            std::string command_usage = "Command: " + it->first;
            std::string command_description = "Description: " + it->second.description + "\n";
            std::string command_argument_list;

            for(u64 i = 0; i < it->second.arguments.size(); i += 1) {
                const command_argument_definition& argument = it->second.arguments[i];
                std::string argument_type_name = command_argument_type_string(argument.type);

                command_usage += " <" + argument.name + ">";
                command_argument_list += "    Argument: <" + argument.name + ">\n" +
                                         "    Type: " + argument_type_name + "\n" +
                                         "    Description: " + argument.description + "\n\n";
            }

            for(auto option : it->second.options) {
                const std::string& option_name = option.first;
                std::string option_type_name = command_argument_type_string(option.second.type);
                const std::string& option_description = option.second.description;

                command_usage += " -" + option_name + "=<" + option_type_name + ">";
                command_argument_list += "    Option: --" + option_name + "\n" +
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
        std::string command_usage = find_command_it->first;

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

        lexer command_lexer;
        lexer_output output;

        std::optional<command_parsed> command_result = parse_command(&command_lexer, &output,
            input.c_str(), static_cast<u64>(input.length()));

        if(!command_result)
            return false;

        const command_parsed& command = command_result.value();
        command.definition->executor(command);

        return true;
    }

    bool listen_command(lexer* command_lexer)
    {
        std::string input;
        lexer_output output;

        std::getline(std::cin, input);

        std::optional<command_parsed> command_result = parse_command(command_lexer, &output,
            input.c_str(), static_cast<u64>(input.length()));

        if(!command_result)
            return false;

        const command_parsed& command = command_result.value();
        command.definition->executor(command);

        return true;
    }

    std::string command_argument_type_string(command_argument_type argument_type)
    {
        std::string argument_type_string;

        switch(argument_type) {
            default:
                log_error("Not implemented command argument type.");
                argument_type_string = "";
                break;
            case command_argument_type::INTEGER:
                argument_type_string = "Integer";
                break;
            case command_argument_type::DECIMAL:
                argument_type_string = "Decimal";
                break;
            case command_argument_type::STRING:
                argument_type_string = "String";
                break;
        }

        return argument_type_string;
    }

    std::optional<const std::pair<const std::string, command_definition>*> find_command(
        lexer_output* output)
    {
        if(output->tokens.size() == 0) {
            // User entered nothing or whitespace only.
            std::cout << "Expected command name." << std::endl;
            return std::optional<const std::pair<const std::string, command_definition>*>{};
        }
        token *command_token = &output->tokens[0];

        if(command_token->type != TOKEN_IDENTIFIER) {
            // User did not enter a valid command name.
            std::cout << "Expected command name." << std::endl;
            return std::optional<const std::pair<const std::string, command_definition>*>{};
        }

        const std::string& command_name = output->identifiers[command_token->value_index];

        auto find_command_it = COMMANDS.find(command_name);
        if(find_command_it == COMMANDS.end()) {
            // User entered a command that does not exist.
            std::cout << "Command \"" << command_name << "\" does not exist." << std::endl;
            return std::optional<const std::pair<const std::string, command_definition>*>{};
        }

        return std::optional<const std::pair<const std::string, command_definition>*>{ &(*find_command_it) };
    }

    std::optional<command_parsed> parse_command(lexer* command_lexer, lexer_output* output,
        const char* input, u64 input_size)
    {
        // Tokenize the input.
        command_lexer->tokenize(input, input_size, output);
        if(output->messages.errors.size() != 0) {
            std::cout << "Failed to tokenize command." << std::endl;
            return std::optional<command_parsed>{};
        }

        // Find the command.
        auto find_result = find_command(output);
        if(!find_result) {
            return std::optional<command_parsed>{};
        }

        command_parsed command;
        command.name = &find_result.value()->first;
        command.definition = &find_result.value()->second;

        u64 token_index = 1;

        // Parse arguments.
        for(u64 i = 0; i < command.definition->arguments.size(); i += 1) {
            if(token_index >= output->tokens.size()) {
                std::cout << "Missing argument(s)." << std::endl;
                return std::optional<command_parsed>{};
            }

            auto argument_result = parse_command_argument(&token_index,
                output, command.definition->arguments[i].type);

            if(!argument_result)
                return std::optional<command_parsed>{};

            command.parsed_arguments.push_back(argument_result.value());
        }

        // Parse optional arguments.
        while(token_index < output->tokens.size()) {
            auto option_result = parse_command_option(&token_index,
                output, command.definition->options);

            if(!option_result)
                return std::optional<command_parsed>{};

            command.parsed_options.push_back(option_result.value());
        }

        return std::optional<command_parsed>{ command };
    }

    std::optional<command_argument_pair> parse_command_argument(u64* token_index, lexer_output* output,
        command_argument_type argument_type)
    {
        command_argument_value value;

        token* current_token = &output->tokens[*token_index];
        *token_index += 1;

        switch(argument_type) {
            default:
                // Not implemented argument type.
                return std::optional<command_argument_pair>{};

            case command_argument_type::INTEGER:
                if(current_token->type != TOKEN_INTEGER) {
                    std::cout << "Mismatched type, expected an integer." << std::endl;
                    return std::optional<command_argument_pair>{};
                }

                value.integer = static_cast<s64>(std::stoll(output->integers[current_token->value_index]));
                return std::optional<command_argument_pair>{
                    command_argument_pair{ command_argument_type::INTEGER, value }
                };

            case command_argument_type::DECIMAL:
                if(current_token->type != TOKEN_DECIMAL) {
                    std::cout << "Mismatched type, expected a decimal." << std::endl;
                    return std::optional<command_argument_pair>{};
                }

                value.decimal = static_cast<f64>(std::stod(output->decimals[current_token->value_index]));
                return std::optional<command_argument_pair>{
                    command_argument_pair{ command_argument_type::DECIMAL, value }
                };

            case command_argument_type::STRING:
                if(current_token->type != TOKEN_STRING) {
                    std::cout << "Mismatched type, expected a string." << std::endl;
                    return std::optional<command_argument_pair>{};
                }

                value.str = output->strings[current_token->value_index].c_str();
                return std::optional<command_argument_pair>{
                    command_argument_pair{ command_argument_type::STRING, value }
                };
        }
    }

    std::optional<command_option_tuple> parse_command_option(u64* token_index, lexer_output* output,
        const std::map<std::string, command_option_definition>& options)
    {
        if(*token_index + 4 >= output->tokens.size()) {
            std::cout << "Incomplete option." << std::endl;
            return std::optional<command_option_tuple>{};
        }

        // Relevant tokens of the option "-identifier=".
        token* dash_token = &output->tokens[*token_index];
        token* dash_token_2 = &output->tokens[*token_index + 1];
        token* identifier_token = &output->tokens[*token_index + 2];
        token* equals_token = &output->tokens[*token_index + 3];
        *token_index += 4;

        if(dash_token->type != '-') {
            std::cout << "Invalid option format, expected \"--\"." << std::endl;
            return std::optional<command_option_tuple>{};
        }
        if(dash_token_2->type != '-') {
            std::cout << "Invalid option format, expected \"--\"." << std::endl;
            return std::optional<command_option_tuple>{};
        }
        if(identifier_token->type != TOKEN_IDENTIFIER) {
            std::cout << "Invalid option format, expected identifier." << std::endl;
            return std::optional<command_option_tuple>{};
        }
        if(equals_token->type != '=') {
            std::cout << "Invalid option format, expected \"=\"." << std::endl;
            return std::optional<command_option_tuple>{};
        }

        const std::string& option_name = output->identifiers[identifier_token->value_index];
        auto option_find_it = options.find(option_name);

        if(option_find_it == options.end()) {
            std::cout << "Option not found for this command." << std::endl;
            return std::optional<command_option_tuple>{};
        }

        const command_argument_type& option_type = option_find_it->second.type;

        // Parse the value
        auto value_result = parse_command_argument(token_index, output, option_type);
        if(!value_result)
            return std::optional<command_option_tuple>{};

        *token_index += 1;
        return std::optional<command_option_tuple>{
            command_option_tuple{
                value_result.value().first, value_result.value().second, &option_name
            }
        };
    }
}