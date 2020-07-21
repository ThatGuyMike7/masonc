#ifndef $_MASON_COMMAND_HPP_$
#define $_MASON_COMMAND_HPP_$

#include "common.hpp"
#include "lexer.hpp"

#include <unordered_map>
#include <vector>
#include <string>
#include <string_view>
#include <utility>

namespace masonc
{
    enum class command_argument_type : u8
    {
        INTEGER,
        DECIMAL,
        STRING
    };
    
    union command_argument_value
    {
        s64 integer;
        f64 decimal;
        const char* str;
    };
    
    struct command_argument_definition
    {
        std::string name;
        std::string description;
        command_argument_type type;
    };
    
    struct command_parsed;
    struct command_definition
    {
        std::string description;
        void(*executor)(const command_parsed& command);
        std::vector<command_argument_definition> arguments;
        std::unordered_map<std::string, command_argument_type> options;
    };

    using command_argument_pair = std::pair<command_argument_type, command_argument_value>;
    using command_option_tuple = std::tuple<command_argument_type,
        command_argument_value, std::string_view>;
    
    struct command_parsed
    {
        std::string_view name;
        const command_definition* definition;
        std::vector<command_argument_pair> parsed_arguments;
        std::vector<command_option_tuple> parsed_options;
    };
    
    // Wait until the user enters something into the input stream,
    // parse the input string into a command and execute it.
    // Returns false if the input cannot be parsed and executed.
    bool listen_command();
    
    result<command_parsed> parse_command(const char* input, u64 input_size);
    
    // Parse non-optional argument of a command
    result<command_argument_pair> parse_command_argument(u64* token_index, lexer_output* output,
        command_argument_type argument_type);
    
    // Parse optional argument of a command
    result<command_option_tuple> parse_command_option(u64* token_index, lexer_output* output,
        const std::unordered_map<std::string, command_argument_type>* options);
    
    void execute_command_question_mark(const command_parsed& command);
    void execute_command_help(const command_parsed& command);
    
    const std::unordered_map<std::string, command_definition> COMMANDS = 
    {
        { 
            "?", 
            command_definition
            {
                "Print a list of all available commands",
                &execute_command_question_mark
            } 
        },
        {
            "help",
            command_definition
            {
                "Print description, usage, and options of a specific command",
                &execute_command_help,
                std::vector<command_argument_definition>
                {
                    command_argument_definition
                    {
                        "command_name", "Name of the command", command_argument_type::STRING
                    }
                }
            }
        }
    };
}

#endif