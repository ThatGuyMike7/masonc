#include "common.hpp"
#include "io.hpp"
#include "lexer.hpp"
#include "parser.hpp"
#include "log.hpp"
#include "lang.hpp"
#include "scope.hpp"
#include "timer.hpp"
#include "test.hpp"
#include "llvm_converter.hpp"
#include "command.hpp"

#include <iostream>
#include <cstdlib>
#include <string>
#include <optional>

constexpr char MASON_ASCII_ART[] = R"(_______ _________ ____________________ 
__  __ `__ \  __ `/_  ___/  __ \_  __ \
_  / / / / / /_/ /_(__  )/ /_/ /  / / /
/_/ /_/ /_/\__,_/ /____/ \____//_/ /_/)";

using namespace masonc;

void perform_tests()
{
    auto parse_tests_pass = test_parse_in_directory("tests/pass", true);
    for(u64 i = 0; i < parse_tests_pass.matched_expected.size(); i += 1) {
        if(!parse_tests_pass.matched_expected[i]) {
            log_error(std::string{
                "parse test failed (expected success): " + parse_tests_pass.files[i]
            }.c_str());
            
            parse_tests_pass.message_lists[i].print_errors();
        }
    }
    
    auto parse_tests_fail = test_parse_in_directory("tests/fail", false);
    for(u64 i = 0; i < parse_tests_fail.matched_expected.size(); i += 1) {
        if(!parse_tests_fail.matched_expected[i]) {
            log_error(std::string{
                "parse test succeeded (expected failure): " + parse_tests_fail.files[i]
            }.c_str());
        }
    }
}

int main(int argc, char** argv)
{
    initialize_language();
    initialize_llvm_converter();
    
    perform_tests();

    // NOTE: It is apparently implementation-defined whether or not the first argument of `argv`
    //       is the program name, but almost everyone passes the program name there.
    std::string command_line_input;
    
    // Ignore the program name and string together the rest of the input.
    for(int i = 1; i < argc; i += 1) {
        command_line_input += argv[i];
        command_line_input += " ";
    }

    lexer command_lexer;
    
    // If `command_line_input` is empty, a user has probably launched the compiler manually.
    if(command_line_input.empty()) {
        std::cout << MASON_ASCII_ART << "\n\n" 
                  << "Compiler for the mason programming language." << "\n"
                  << "Written by Mike Jasinski." << "\n"
                  << "github.com/ThatGuyMike7/masonc" << "\n\n"
        
                  << "Usage: command [arguments] [options]?" << "\n"
                  << "Type \"help\" for a list of commands." << "\n\n"
                  << std::flush;
        
        auto test_compile_command =
            R"(build "E:/mason-lang/projects/masonc/tests/mason/*" "main.o")";

        execute_command(test_compile_command);

        while(true) {
            listen_command(&command_lexer);
        }
    }
    // The compiler was invoked with specific command line arguments.
    else {
        lexer_output output;

        // Parse and execute the command in question.
        auto command_result = parse_command(&command_lexer, &output,
            command_line_input.c_str(), command_line_input.length());
        
        if(command_result)
            command_result.value().definition->executor(command_result.value());
    }
    
    return 0;
}
