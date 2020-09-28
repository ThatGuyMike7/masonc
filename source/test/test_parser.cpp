#include <test_parser.hpp>

#include <lexer.hpp>
#include <parser.hpp>
#include <log.hpp>

namespace masonc::test::parser
{
    test_parse_in_directory_output test_parse_in_directory(const char* directory_path, bool expected)
    {
        test_parse_in_directory_output output;
        output.files = directory_files_recurse(directory_path);

        for (masonc::u64 i = 0; i < output.files.size(); i += 1) {
            auto test_parse_result = test_parse(output.files[i].c_str());

            if(!test_parse_result) {
                log_error("Cannot perform test");
                output.matched_expected.emplace_back(true);
                output.message_lists.emplace_back(masonc::message_list{});
                continue;
            }

            if(expected == false) {
                output.matched_expected.emplace_back(test_parse_result.value().errors.size() > 0);
                output.message_lists.emplace_back(test_parse_result.value());
            }
            else {
                output.matched_expected.emplace_back(test_parse_result.value().errors.size() == 0);
                output.message_lists.emplace_back(test_parse_result.value());
            }
        }

        return output;
    }

    std::optional<masonc::message_list> test_parse(const char* filename)
    {
        masonc::u64 file_length;
        std::optional<char*> file;

        file = file_read(filename, 1024, &file_length);
        if (!file)
            return std::optional<masonc::message_list>{};

        masonc::lexer lex;
        masonc::lexer_output lex_output;

        lex.tokenize(file.value(), file_length, &lex_output);
        if (lex_output.messages.errors.size() > 0)
            return std::optional<masonc::message_list>{};

        masonc::parser par;
        masonc::parser_output par_output;

        par.parse(&lex_output, &par_output);
        std::free(file.value());

        return std::optional<masonc::message_list>{ par_output.messages };
    }
}