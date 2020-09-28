#ifndef $_MASONC_TEST_PARSER_HPP_$
#define $_MASONC_TEST_PARSER_HPP_$

#include <message.hpp>

#include <vector>
#include <optional>
#include <string>

namespace masonc::test::parser
{
    struct test_parse_in_directory_output
    {
        std::vector<std::string> files;
        std::vector<bool> matched_expected;
        std::vector<message_list> message_lists;
    };

    // Returns a list of file paths, messages and booleans specifying if test-parsing them
    // matches the "expected" value.
    //
    // All files in "directory_path" and its sub-directories are tested individually.
    test_parse_in_directory_output test_parse_in_directory(const char* directory_path, bool expected);

    // Returns empty result if file i/o or lexing failed.
    // Returns the parser's message list otherwise.
    std::optional<message_list> test_parse(const char* filename);
}

#endif