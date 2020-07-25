#include "command.hpp"

#include "log.hpp"

#include <iostream>

namespace masonc
{
    void execute_command_help(const command_parsed& command)
    {
        // TODO: Create and sort pointers to key-value pairs in `COMMANDS`
        //       by `command_definition::order` to get them printed in the same order
        //       in which they are defined.
        
        std::string output;
        
        for(auto it = COMMANDS.begin(); it != COMMANDS.end(); it++)
        {
            std::string command_name = "Command: " + it->first + "\n";
            std::string command_usage = "Usage: " + it->first;
            std::string command_description = "Description: " + it->second.description + "\n";
            std::string command_argument_list;
            
            for(u64 i = 0; i < it->second.arguments.size(); i += 1)
            {
                const command_argument_definition& argument = it->second.arguments[i];
                std::string argument_type_name = command_argument_type_string(argument.type);
                
                command_usage += " <" + argument.name + ">";
                command_argument_list += "    Argument: " + argument.name + "\n" +
                                         "    Type: " + argument_type_name + "\n" +
                                         "    Description: " + argument.description + "\n\n";
            }
            
            for(auto option : it->second.options)
            {
                const std::string& option_name = option.first;
                std::string option_type_name = command_argument_type_string(option.second.type);
                const std::string& option_description = option.second.description;
                
                command_usage += " -" + option_name + "=<" + option_type_name + ">";
                command_argument_list += "    Option: " + option_name + "\n" +
                                       //"    Type: " + option_type_name + "\n" +
                                         "    Description: " + option_description + "\n\n";
            }
            
            command_usage += "\n";
            command_description += "\n";
            command_argument_list += "\n";
            
            output += command_name + command_usage + command_description + command_argument_list;
        }
        
        std::cout << "\n" << output << std::flush;
    }
    
    void execute_command_usage(const command_parsed& command)
    {
        std::cout << "usage" << std::endl;
    }
    
    void execute_command_compile(const command_parsed& command)
    {
        std::cout << "compile" << std::endl;
    }
    
    std::string command_argument_type_string(command_argument_type argument_type)
    {
        std::string argument_type_string;
        
        switch(argument_type)
        {
            default:
                log_error("Not implemented command argument type");
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
    
    bool listen_command()
    {
        std::string input;
        std::getline(std::cin, input);
        
        result<command_parsed> command_result = parse_command(
            input.c_str(), static_cast<u64>(input.length()));
        
        if(!command_result)
            return false;
        
        const command_parsed& command = command_result.value();
        command.definition->executor(command);
        
        return true;
    }
    
    result<command_parsed> parse_command(const char* input, u64 input_size)
    {
        lexer command_lexer;
        lexer_output output;
        
        command_lexer.tokenize(input, input_size, &output);
        if(output.messages.errors.size() != 0)
        {
            std::cout << "Failed to tokenize command" << std::endl;
            return result<command_parsed>{};
        }
        
        if(output.tokens.size() == 0)
        {
            // User entered nothing or whitespace only
            std::cout << "Expected command name" << std::endl;
            return result<command_parsed>{};
        }
        
        token* first_token = &output.tokens[0];
        if(first_token->type != TOKEN_IDENTIFIER)
        {
            // User did not enter a valid command name
            std::cout << "Expected command name" << std::endl;
            return result<command_parsed>{};
        }
        
        const std::string& command_name = output.identifiers[first_token->value_index];
        
        auto find_command_it = COMMANDS.find(command_name);
        if(find_command_it == COMMANDS.end())
        {
            // User entered a command that does not exist
            std::cout << "Command does not exist" << std::endl;
            return result<command_parsed>{};
        }
        
        const command_definition& definition = find_command_it->second;
        u64 token_index = 1;
        
        command_parsed command;
        command.name = std::string_view{ find_command_it->first.data(),
            find_command_it->first.length() };
        
        command.definition = &definition;
        
        // Parse arguments
        for(u64 i = 0; i < definition.arguments.size(); i += 1)
        {
            if(token_index >= output.tokens.size())
            {
                std::cout << "Missing argument(s)" << std::endl;
                return result<command_parsed>{};
            }
            
            auto argument_result = parse_command_argument(&token_index,
                &output, definition.arguments[i].type);
            
            if(!argument_result)
                return result<command_parsed>{};
            
            command.parsed_arguments.push_back(argument_result.value());
        }
        
        // Parse optional arguments
        while(token_index < output.tokens.size())
        {
            auto option_result = parse_command_option(&token_index,
                &output, definition.options);
            
            if(!option_result)
            {
                return result<command_parsed>{};
            }
            
            command.parsed_options.push_back(option_result.value());
        }
        
        return result<command_parsed>{ command };
    }
    
    result<command_argument_pair> parse_command_argument(u64* token_index, lexer_output* output,
        command_argument_type argument_type)
    {
        command_argument_value value;

        token* current_token = &output->tokens[*token_index];
        *token_index += 1;

        if(argument_type == command_argument_type::INTEGER)
        {
            if(current_token->type != TOKEN_INTEGER)
            {
                std::cout << "Mismatched type, expected an integer" << std::endl;
                return result<command_argument_pair>{};
            }
            
            value.integer = static_cast<s64>(std::stoll(output->integers[current_token->value_index]));
            return result<command_argument_pair>{
                command_argument_pair{ command_argument_type::INTEGER, value }
            };
        }
        else if(argument_type == command_argument_type::DECIMAL)
        {
            if(current_token->type != TOKEN_DECIMAL)
            {
                std::cout << "Mismatched type, expected a decimal" << std::endl;
                return result<command_argument_pair>{};
            }
            
            value.decimal = static_cast<f64>(std::stod(output->decimals[current_token->value_index]));
            return result<command_argument_pair>{ 
                command_argument_pair{ command_argument_type::DECIMAL, value }
            };
        }
        else if(argument_type == command_argument_type::STRING)
        {
            if(current_token->type != TOKEN_STRING)
            {
                std::cout << "Mismatched type, expected a string" << std::endl;
                return result<command_argument_pair>{};
            }
            
            value.str = output->strings[current_token->value_index].c_str();
            return result<command_argument_pair>{ 
                command_argument_pair{ command_argument_type::STRING, value }
            };
        }
        
        // Not implemented argument type
        return result<command_argument_pair>{};
    }
    
    result<command_option_tuple> parse_command_option(u64* token_index, lexer_output* output,
        const std::map<std::string, command_option_definition>& options)
    {
        if(*token_index + 3 >= output->tokens.size())
        {
            std::cout << "Incomplete option" << std::endl;
            return result<command_option_tuple>{};
        }
        
        // Relevant tokens of the option: "-identifier="
        token* first_token = &output->tokens[*token_index];
        token* second_token = &output->tokens[*token_index + 1];
        token* third_token = &output->tokens[*token_index + 2];
        *token_index += 3;
        
        if(first_token->type != '-')
        {
            std::cout << "Invalid option format, expected '-'" << std::endl;
            return result<command_option_tuple>{};
        }
        if(second_token->type != TOKEN_IDENTIFIER)
        {
            std::cout << "Invalid option format, expected identifier" << std::endl;
            return result<command_option_tuple>{};
        }
        if(third_token->type != '=')
        {
            std::cout << "Invalid option format, expected '='" << std::endl;
            return result<command_option_tuple>{};
        }
        
        const std::string& option_name = output->identifiers[second_token->value_index];
        std::string_view option_name_view{ option_name.data(), option_name.length() };
        
        // Find the option type by name
        auto option_find_it = options.find(option_name);
        if(option_find_it == options.end())
        {
            std::cout << "Option not found for this command" << std::endl;
            return result<command_option_tuple>{};
        }
        
        const command_argument_type& option_type = option_find_it->second.type;
        
        // Parse the value
        auto value_result = parse_command_argument(token_index, output, option_type);
        if(!value_result)
            return result<command_option_tuple>{};
        
        *token_index += 1;
        return result<command_option_tuple>{
            command_option_tuple{ 
                value_result.value().first, value_result.value().second, option_name_view
            }
        };
    }
}