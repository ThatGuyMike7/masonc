#include "command.hpp"

#include <iostream>

namespace masonc
{
    void parse_command(const char* input, u64 input_size)
    {
        lexer cmd_lexer;
        lexer_output* output = new lexer_output;
        
        cmd_lexer.tokenize(input, input_size, output);
        if(output->messages.errors.size() != 0)
        {
            std::cout << "Failed to tokenize command" << std::endl;
            return;
        }
        
        token* first_token = &output->tokens[0];
        if(first_token->type != TOKEN_IDENTIFIER)
        {
            std::cout << "Expected command name" << std::endl;
            return;
        }
        
        const std::string& command_name = output->identifiers[first_token->value_index];
        
        auto find_command_it = COMMANDS.find(command_name);
        if(find_command_it == COMMANDS.end())
        {
            std::cout << "Command does not exist" << std::endl;
            return;
        }
        
        const command& cmd = find_command_it->second;
        u64 token_index = 1;
        
        std::vector<command_argument_pair> parsed_arguments;
        std::vector<command_optional_tuple> parsed_optionals;

        // Parse arguments
        for(u64 i = 0; i < cmd.arguments.size(); i += 1)
        {
            if(token_index >= output->tokens.size())
            {
                std::cout << "Missing argument(s)" << std::endl;
                return;
            }
            
            auto argument_result = parse_command_argument(&token_index, output, &cmd.arguments[i]);
            if(!argument_result)
                return;
            
            parsed_arguments.push_back(argument_result.value());
        }
        
        // Parse optional arguments
        while(token_index < output->tokens.size())
        {
            auto option_result = parse_command_option(&token_index, output, &cmd.options);
            if(!option_result)
                return;
            
            parsed_optionals.push_back(option_result.value());
        }
        
        // TODO: Return the output
        
        delete output;
    }
    
    result<command_argument_pair> parse_command_argument(u64* token_index,
        lexer_output* output, const command_argument* argument)
    {
        command_argument_value value;
        
        if(argument->type == command_argument_type::INTEGER)
        {
            if(output->tokens[*token_index].type != TOKEN_INTEGER)
            {
                std::cout << "Mismatched type, expected an integer" << std::endl;
                return result<command_argument_pair>{};
            }
            
            value.integer = static_cast<s64>(std::stoll(output->integers[*token_index]));
            
            return result<command_argument_pair>{
                command_argument_pair{ command_argument_type::INTEGER, value }
            };
        }
        else if(argument->type == command_argument_type::DECIMAL)
        {
            if(output->tokens[*token_index].type != TOKEN_DECIMAL)
            {
                std::cout << "Mismatched type, expected a decimal" << std::endl;
                return result<command_argument_pair>{};
            }
            
            value.decimal = static_cast<f64>(std::stod(output->decimals[*token_index]));
            
            return result<command_argument_pair>{ 
                command_argument_pair{ command_argument_type::DECIMAL, value }
            };
        }
        else if(argument->type == command_argument_type::STRING)
        {
            if(output->tokens[*token_index].type != TOKEN_STRING)
            {
                std::cout << "Mismatched type, expected a string" << std::endl;
                return result<command_argument_pair>{};
            }
            
            value.str = output->strings[*token_index].c_str();

            return result<command_argument_pair>{ 
                command_argument_pair{ command_argument_type::STRING, value }
            };
        }
        
        *token_index += 1; 
    }
    
    result<command_optional_tuple> parse_command_option(u64* token_index,
        lexer_output* output, const command_argument* option)
    {
        if(*token_index + 3 >= output->tokens.size())
        {
            std::cout << "Incomplete option" << std::endl;
            return result<command_optional_tuple>{};
        }
        
        token& first_token = output->tokens[*token_index];
        token& second_token = output->tokens[*token_index + 1];
        token& third_token = output->tokens[*token_index + 2];
        *token_index += 3;
        
        if(first_token.type != '-')
        {
            std::cout << "Wrong option format, expected '-'" << std::endl;
            return result<command_optional_tuple>{};
        }
        if(second_token.type != TOKEN_IDENTIFIER)
        {
            std::cout << "Wrong option format, expected identifier" << std::endl;
            return result<command_optional_tuple>{};
        }
        if(third_token.type != '=')
        {
            std::cout << "Wrong option format, expected '='" << std::endl;
            return result<command_optional_tuple>{};
        }
        
        const std::string& name = output->identifiers[second_token.value_index];
        std::string_view name_view{ name.data(), name.length() };
        
        auto value_result = parse_command_argument(token_index, output, option);
        if(!value_result)
            return result<command_optional_tuple>{};
        
        *token_index += 1;
        
        return result<command_optional_tuple>{
            command_optional_tuple{ 
                value_result.value().first, value_result.value().second, name_view
            }
        };
    }
}