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

constexpr char MASON_ASCII_ART[] = R"(_______ _________ ____________________ 
__  __ `__ \  __ `/_  ___/  __ \_  __ \
_  / / / / / /_/ /_(__  )/ /_/ /  / / /
/_/ /_/ /_/\__,_/ /____/ \____//_/ /_/)";

using namespace masonc;

int main(int argc, char** argv)
{
	//test_string_collection_against_vector_iteration_and_append_speed();
	//std::getchar();
	//return 0;
	
	initialize_language();
	initialize_llvm_converter();
	
	/*
	if (!test_parser("tests/pass/pass_1.mason")) log_error("Test pass_1 failed");
	if (!test_parser("tests/pass/pass_2.mason")) log_error("Test pass_2 failed");
	if (!test_parser("tests/pass/pass_3.mason")) log_error("Test pass_3 failed");
	
	if (test_parser("tests/error/error_1.mason")) log_error("Test error_1 did not fail");
	if (test_parser("tests/error/error_2.mason")) log_error("Test error_2 did not fail");
	if (test_parser("tests/error/error_3.mason")) log_error("Test error_3 did not fail");
	if (test_parser("tests/error/error_4.mason")) log_error("Test error_4 did not fail");
	if (test_parser("tests/error/error_5.mason")) log_error("Test error_5 did not fail");
	if (test_parser("tests/error/error_6.mason")) log_error("Test error_6 did not fail");
	if (test_parser("tests/error/error_7.mason")) log_error("Test error_7 did not fail");
	if (test_parser("tests/error/error_8.mason")) log_error("Test error_8 did not fail");
	if (test_parser("tests/error/error_9.mason")) log_error("Test error_9 did not fail");
	if (test_parser("tests/error/error_10.mason")) log_error("Test error_10 did not fail");
	if (test_parser("tests/error/error_11.mason")) log_error("Test error_11 did not fail");
	
	getchar();
	return 0;
	*/
	
	// NOTE: It is apparently implementation-defined whether or not the first argument of `argv`
	//		 is the program name, but almost everyone passes the program name there.
	std::string command_line_input;
	
	// Ignore the program name and string together the rest of the input
	for(int i = 1; i < argc; i += 1)
	{
		command_line_input += argv[i];
		command_line_input += " ";
	}

	lexer command_lexer;
	
	// If `command_line_input` is empty, a user has probably launched the compiler manually.
	if(command_line_input.empty())
	{
		std::cout << MASON_ASCII_ART << "\n\n" 
				  << "mason programming language compiler" << "\n"
				  << "written by Mike Jasinski" << "\n"
				  << "github.com/ThatGuyMike7/masonc" << "\n\n"
		
				  << "Usage: command [arguments] [options]?" << "\n"
				  << "Type \"help\" for a list of commands" << "\n\n"
				  << std::flush;
		
		while(true)
		{
			listen_command(&command_lexer);
		}
	}
	// The compiler was invoked with specific command line arguments
	else
	{
		lexer_output output;

		// Parse and execute the command in question
		auto command_result = parse_command(&command_lexer, &output,
			command_line_input.c_str(), command_line_input.length());
		
		if(command_result)
			command_result.value().definition->executor(command_result.value());
	}
	
	return 0;
	
	/*
	timer timer_read_file;
	timer_read_file.start();
	
	u64 file_length;
	result<char*> file_result = file_read("tests/test.mason", 1024, &file_length);
	if(!file_result)
	{
		log_error("Something went wrong reading file");
		std::getchar();
		return 0;
	}
	
	timer_read_file.end();
	log_message(std::string{ std::to_string(file_length) + " characters read from source file in " + timer_read_file.get_formatted_duration() }.c_str());
	
	lexer lex;
	lexer_output* lex_output = new lexer_output;
	
	lex.tokenize(file_result.value(), file_length, lex_output);
	if(lex_output->messages.errors.size() > 0)
	{
		log_error("Something went wrong tokenizing");
		std::getchar();
		return 0;
	}
	//lexer.print_tokens();
	//getchar();
	
	parser par;
	parser_output* par_output = new parser_output;
	
	par.parse(lex_output, par_output);
	par.print_expressions();
	par_output->messages.print();
	
	llvm_converter converter;
	converter.generate(par_output);
	converter.print_IR();

	std::cout << "\nDone" << std::endl;
	
	converter.free();
	delete par_output;
	delete lex_output;
	std::free(file_result.value());
	
	getchar();
	return 0;
	*/
}