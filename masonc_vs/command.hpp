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
    
    using command_argument_pair = std::pair<command_argument_type, command_argument_value>;
    using command_optional_tuple = std::tuple<command_argument_type,
        command_argument_value, std::string_view>;
    
    struct command_argument
    {
        std::string name;
        command_argument_type type;
    };
    
    struct command
    {
        std::vector<command_argument> arguments;
        std::vector<command_argument> options;
    };
    
    const std::unordered_map<std::string, command> COMMANDS = 
    {
        { "help", command{} }
    };
    
    void parse_command(const char* input, u64 input_size);
    
    // Parse non-optional argument of a command
    result<command_argument_pair> parse_command_argument(u64* token_index,
        lexer_output* output, const command_argument* argument);
    
    // Parse optional argument of a command
    result<command_optional_tuple> parse_command_option(u64* token_index,
        lexer_output* output, const command_argument* option);
}

#endif