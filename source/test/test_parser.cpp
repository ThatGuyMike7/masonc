#include <test_parser.hpp>

#include <lexer.hpp>
#include <parser.hpp>
#include <logger.hpp>
#include <io.hpp>

namespace masonc::test::parser
{
    test_parse_in_directory_output test_parse_in_directory(const char* directory_path, bool expected)
    {
        test_parse_in_directory_output output;
        output.files = directory_files_recurse(directory_path);

        for (masonc::u64 i = 0; i < output.files.size(); i += 1) {
            auto test_parse_result = test_parse(output.files[i].c_str());

            if(!test_parse_result) {
                global_logger.log_error("Cannot perform parse test");
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

        masonc::lexer::lexer_instance lexer;
        masonc::parser::parser_instance_output parser_output;

        lexer.tokenize(file.value(), file_length, &parser_output.lexer_output);
        if (parser_output.lexer_output.messages.errors.size() > 0)
            return std::optional<masonc::message_list>{};

        masonc::parser::parser_instance parser{ &parser_output };

        std::free(file.value());

        return std::optional<masonc::message_list>{ parser_output.messages };
    }
}