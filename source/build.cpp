#include <build.hpp>

#include <log.hpp>
#include <lexer.hpp>
#include <parser.hpp>
#include <llvm_converter.hpp>

#include <cstdlib>

namespace masonc
{
    void build_object(std::vector<path> sources,
        std::unordered_set<std::string> additional_extensions)
    {
        additional_extensions.insert(".m");
        additional_extensions.insert(".mason");

        lexer source_lexer;
        parser source_parser;
        llvm_converter source_converter;

        std::vector<lexer_output> lexer_outputs;
        std::vector<parser_output> parser_outputs;
        std::vector<llvm_converter_output> converter_outputs;

        {
            std::cout << '\n' << "Lexing..." << std::endl;
            for (u64 i = 0; i < sources.size(); i += 1) {
                auto file_paths = files_from_path(sources[i], additional_extensions);

                for (u64 j = 0; j < file_paths.size(); j += 1) {
                    u64 current_file_length;
                    std::optional<char*> current_file = file_read(file_paths[j].c_str(),
                        1024u, &current_file_length);

                    if (!current_file) {
                        //std::free(current_file.value());
                        goto END;
                    }

                    lexer_output* current_lexer_output = &lexer_outputs.emplace_back(lexer_output{});
                    source_lexer.tokenize(current_file.value(),
                        current_file_length, current_lexer_output);

                    std::free(current_file.value());

                    if (current_lexer_output->messages.errors.size() > 0) {
                        current_lexer_output->messages.print_errors();
                        std::cout << std::endl;
                        goto END;
                    }
                }
            }

            std::cout << "Parsing..." << std::endl;
            for (u64 i = 0; i < lexer_outputs.size(); i += 1) {
                lexer_output* current_lexer_output = &lexer_outputs[i];
                parser_output* current_parser_output = &parser_outputs.emplace_back(parser_output{});

                source_parser.parse(current_lexer_output, current_parser_output);

                if (current_parser_output->messages.errors.size() > 0) {
                    current_parser_output->messages.print_errors();
                    std::cout << std::endl;
                    goto END;
                }
            }

            /*
            std::cout << "Generating code..." << std::endl;
            for (u64 i = 0; i < parser_outputs.size(); i += 1) {
                lexer_output* current_lexer_output = &lexer_outputs[i];
                parser_output* current_parser_output = &parser_outputs[i];
                llvm_converter_output* current_converter_output =
                    &converter_outputs.emplace_back(llvm_converter_output{});

                source_converter.convert(current_lexer_output, current_parser_output,
                    current_converter_output);

                if (current_converter_output->messages.errors.size() > 0) {
                    current_converter_output->messages.print_errors();
                    goto END;
                }
            }
            */
        }

        END:
        source_parser.free();
    }

    const std::string build_stage_name(build_stage stage)
    {
        switch(stage)
        {
            default:
                return "";
            case build_stage::LEXER:
                return "Lexer";
            case build_stage::PARSER:
                return "Parser";
            case build_stage::PRE_LINKER:
                return "Pre-Linker";
            case build_stage::LINKER:
                return "Linker";
            case build_stage::CODE_GENERATOR:
                return "Code Generator";
        }
    }
}