#ifndef MASONC_LEXER_HPP
#define MASONC_LEXER_HPP

#include <common.hpp>
#include <location.hpp>
#include <logger.hpp>
#include <message.hpp>
#include <containers.hpp>

#include <vector>
#include <string>
#include <array>
#include <optional>

namespace masonc::lexer
{
    enum token_type : s8
    {
        TOKEN_IDENTIFIER = -1,
        TOKEN_INTEGER = -2,
        TOKEN_DECIMAL = -3,
        TOKEN_STRING = -4,

        // Composed tokens.
        TOKEN_DOUBLECOLON = -5,
        TOKEN_RIGHT_POINTER = -6
    };

    struct token
    {
        // Index of value if type is identifier, integer, decimal.
        u64 value_index;
        s8 type;
    };

    // Two characters that make up a single token.
    constexpr u64 COMPOSED_TOKENS_LENGTH = 4;
    constexpr char COMPOSED_TOKENS[] =
    {
        ':', ':',
        '-', '>'
    };

    const token_type COMPOSED_TOKEN_TYPES[] =
    {
        TOKEN_DOUBLECOLON,
        TOKEN_RIGHT_POINTER
    };

    constexpr char ESCAPE_BEGIN = '\\';

    // Gets a composed token as string by type.
    // Returns an empty string if token is not composed or could not be found.
    std::string get_composed_token(token_type type);

    // Returns true for a valid character after an "ESCAPE_CHAR" in both constant string and char literals.
    // Additionally, '"' is valid in string and "'" in char literals,
    // but this is not checked here and it will return false.
    bool is_escape_sequence(char c);

    // Returns the correct escape character for an escape sequence, including '"' and "'".
    // Returns "\0" for invalid sequences, so check for it prior.
    // For example: Passing "n" will return "\n", passing '"' will simply return '"'.
    char get_escape_char(char sequence);

    bool is_space(char c);

    struct lexer_instance_output
    {
        std::vector<token> tokens;

        // Locations of tokens that might be needed for error reporting.
        std::vector<token_location> locations;

        cstring_collection identifiers;
        cstring_collection integers;
        cstring_collection decimals;
        cstring_collection strings;

        message_list messages;
    };

    struct lexer_instance
    {
        // 'input': Null-terminated string that will be split into tokens.
        // 'input_size': Number of characters in input.
        // 'output' is expected to be allocated and empty.
        // 'tab_size' should be set to get correct error message column numbers.
        //
        // This function can be called multiple times just fine.
        void tokenize(const char* input, u64 input_size, lexer_instance_output* output, u8 tab_size = 4);

        // Print all tokens for debug purposes.
        void print_tokens();

    private:
        const char* input;
        lexer_instance_output* output;

        u64 char_index;
        u64 line_number;

        // The "column_number" always points to one column behind the last character.
        u64 column_number;

        u8 tab_size;

        // Resets the lexer.
        void prepare(const char* input, u64 input_size, lexer_instance_output* output, u8 tab_size);
        void analyze();

        // Returns true if next char is valid and increment "i".
        // Returns false if next char is the null terminator.
        std::optional<char> get_char();

        // Returns true if next char is valid, but do not increment "i".
        // Returns false if next char is the null terminator.
        std::optional<char> peek_char();

        // Utility functions
        void add_identifier(const std::string& identifier, u64 start_column, u64 end_column);
        void add_number(const std::string& number, bool dot, u64 start_column, u64 end_column);
    };

    constexpr std::array<bool, 127> build_alpha_lookup()
    {
        // All values are default-initialized to false.
        std::array<bool, 127> result{};

        // [A-Z]
        for (u64 i = 65; i <= 90; i += 1)
        {
            result[i] = true;
        }

        // [a-z]
        for (u64 i = 97; i <= 122; i += 1)
        {
            result[i] = true;
        }

        // '_'
        result[95] = true;

        return result;
    }

    constexpr std::array<bool, 127> build_num_lookup()
    {
        // All values are default-initialized to false.
        std::array<bool, 127> result{};

        // [0-9]
        for (u64 i = 48; i <= 57; i += 1)
        {
            result[i] = true;
        }

        return result;
    }

    //constexpr std::array<bool, 127> build_alpha_lookup();
    constexpr std::array<bool, 127> ALPHA_LOOKUP = build_alpha_lookup();
    //constexpr std::array<bool, 127> build_num_lookup();
    constexpr std::array<bool, 127> NUM_LOOKUP = build_num_lookup();

    // Is alpha [a-z], [A-Z] or underscore "_".
    constexpr bool is_alpha(s8 c)
    {
        return ALPHA_LOOKUP[c];
    }

    // Is digit [0-9].
    constexpr bool is_num(s8 c)
    {
        return NUM_LOOKUP[c];
    }

    // Is alpha, underscore or digit [a-z], [A-Z], "_", [0-9].
    constexpr bool is_alnum(s8 c)
    {
        return ALPHA_LOOKUP[c] || NUM_LOOKUP[c];
    }
}

#endif