#include <common.hpp>
#include <io.hpp>
#include <lexer.hpp>
#include <parser.hpp>
#include <log.hpp>
#include <language.hpp>
#include <scope.hpp>
#include <timer.hpp>
#include <test.hpp>
#include <llvm_converter.hpp>
#include <command.hpp>
#include <version.hpp>
#include <containers.hpp>
#include <dependency_graph.hpp>

#include <iostream>
#include <cstdlib>
#include <string>
#include <optional>

constexpr char MASON_ASCII_ART[] = R"(_______ _________ ____________________
__  __ `__ \  __ `/_  ___/  __ \_  __ \
_  / / / / / /_/ /_(__  )/ /_/ /  / / /
/_/ /_/ /_/\__,_/ /____/ \____//_/ /_/)";

int main(int argc, char** argv)
{
    // Decouple C++ and C streams and prevent automatic flushing of "std::cout" on
    // every newline when using an interactive device such as a terminal.
    std::ios_base::sync_with_stdio(false);

    // Prevent "std::cin" from automatically flushing "std::cout".
    std::cin.tie(nullptr);

    masonc::initialize_language();
    masonc::initialize_llvm_converter();

    masonc::test::perform_all_tests();

    // NOTE: It is apparently implementation-defined whether or not the first argument of "argv"
    //       is the program name, but almost everyone passes the program name there.
    std::string command_line_input;

    // Ignore the program name and string together the rest of the input.
    for(int i = 1; i < argc; i += 1) {
        command_line_input += argv[i];
        command_line_input += " ";
    }

    masonc::lexer command_lexer;

    // If "command_line_input" is empty, a user has probably launched the compiler manually.
    if(command_line_input.empty()) {
        std::cout << MASON_ASCII_ART << "\n\n"
                  << "Compiler for the mason programming language, "
                  << "written by Mike Jasinski." << "\n"
                  << "Version " << masonc::VERSION << "\n"
                  << "Ongoing development at https://github.com/ThatGuyMike7/masonc" << "\n\n"

                  << "Usage: command [arguments] [options]?" << "\n"
                  << "Type \"help\" for a list of commands." << "\n\n"
                  << std::flush;

        auto test_compile_command =
            R"(build "E:/mason-lang/projects/masonc/tests/mason/*" "main.o")";

        masonc::execute_command(test_compile_command);

        while(true) {
            masonc::listen_command(&command_lexer);
        }
    }
    // The compiler was invoked with specific command line arguments.
    else {
        masonc::lexer_output output;

        // Parse and execute the command in question.
        auto command_result = masonc::parse_command(&command_lexer, &output,
            command_line_input.c_str(), command_line_input.length());

        if(command_result)
            command_result.value().definition->executor(command_result.value());
    }

    return 0;
}
