#ifndef $_MASON_COMMAND_HPP_$
#define $_MASON_COMMAND_HPP_$

#include <common.hpp>
#include <lexer.hpp>

#include <map>
#include <vector>
#include <string>
#include <string_view>
#include <utility>
#include <optional>

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

    struct command_option_definition
    {
        std::string description;
        command_argument_type type;
    };

    struct command_parsed;
    struct command_definition
    {
        u64 order;
        std::string description;
        void(*executor)(const command_parsed& command);
        std::vector<command_argument_definition> arguments;
        std::map<std::string, command_option_definition> options;
    };

    using command_argument_pair = std::pair<command_argument_type, command_argument_value>;
    using command_option_tuple = std::tuple<command_argument_type,
        command_argument_value, const std::string*>;

    struct command_parsed
    {
        const std::string* name;
        const command_definition* definition;
        std::vector<command_argument_pair> parsed_arguments;
        std::vector<command_option_tuple> parsed_options;
    };

    void execute_command_help(const command_parsed& command);
    void execute_command_usage(const command_parsed& command);
    void execute_command_exit(const command_parsed& command);
    void execute_command_build(const command_parsed& command);

    // Parse "input" into a command and execute it.
    // Returns false if the input cannot be parsed nor executed.
    // This function constructs its own "lexer" object and destructs it at the end.
    bool execute_command(const std::string& input);

    // Wait until the user enters something into the input stream,
    // parse the string into a command and execute it.
    // Returns false if the input cannot be parsed nor executed.
    bool listen_command(lexer* command_lexer);

    // Get a "command_argument_type" value as string.
    std::string command_argument_type_string(command_argument_type argument_type);

    // Returns a pointer to a key-value pair in "COMMANDS"
    // that matches the command name found in the tokenizer's output.
    static std::optional<const std::pair<const std::string, command_definition>*> find_command(
        lexer_output* output);

    std::optional<command_parsed> parse_command(lexer* command_lexer, lexer_output* output,
        const char* input, u64 input_size);

    // Parse non-optional argument of a command.
    std::optional<command_argument_pair> parse_command_argument(u64* token_index, lexer_output* output,
        command_argument_type argument_type);

    // Parse optional argument of a command.
    std::optional<command_option_tuple> parse_command_option(u64* token_index, lexer_output* output,
        const std::map<std::string, command_option_definition>& options);

    inline const std::map<std::string, command_definition> COMMANDS =
    {
        {
            "help",
            command_definition {
                0,
                "Print a list of all available commands.",
                &execute_command_help
            }
        },
        {
            "usage",
            command_definition {
                1,
                "Print description and usage of a specific command.",
                &execute_command_usage,
                std::vector<command_argument_definition>
                {
                    command_argument_definition {
                        "command_name",
                        "Name of the command.",
                        command_argument_type::STRING
                    }
                }
            }
        },
        {
            "exit",
            command_definition {
                2,
                "Quit the program.",
                &execute_command_exit
            }
        },
        {
            "build",
            command_definition {
                3,
                "Build a list of source files into an object file.",
                &execute_command_build,
                std::vector<command_argument_definition>
                {
                    command_argument_definition {
                        "sources",
                        "List of source directory and/or "
                        "file paths separated by \"\\n\"."
                        "\n                 "
                        "Directory paths must end with \"/\". For recursively including "
                        "all sub-directories,"
                        "\n                 "
                        "the path must end with \"/*\". ",
                        command_argument_type::STRING
                    },
                    command_argument_definition {
                        "object_file_name",
                        "Name of the output object file.",
                        command_argument_type::STRING
                    }
                },
                std::map<std::string, command_option_definition>
                {
                    {
                        "add_extensions",
                        command_option_definition {
                            "Additional file extensions to consider source files "
                            "separated by \"\\n\".",
                            command_argument_type::STRING
                        }
                    }
                }
            }
        }
    };
}

#endif