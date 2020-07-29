#include "test.hpp"
#include "common.hpp"
#include "lexer.hpp"
#include "parser.hpp"
#include "io.hpp"
#include "scope.hpp"
#include "lang.hpp"
#include "string_collection.hpp"

#include <cstdlib>
#include <chrono>
#include <iostream>

namespace masonc
{
	test_parse_in_directory_output test_parse_in_directory(const char* directory_path, bool expected)
	{
		test_parse_in_directory_output output;
		output.files = directory_files_recursive(directory_path);
		for(u64 i = 0; i < output.files.size(); i += 1) {
			output.matched_expected.emplace_back(test_parse(output.files[i].c_str()) == expected);
		}
		return output;
	}
	
	bool test_parse(const char* filename)
	{
		u64 file_length;
		result<char*> file;
		
		file = file_read(filename, 1024, &file_length);
		if (!file)
			return false;

		lexer lex;
		lexer_output lex_output;
		
		lex.tokenize(file.value(), file_length, &lex_output);
		if (lex_output.messages.errors.size() > 0)
			return false;

		parser par;
		parser_output par_output;
		
		par.parse(&lex_output, &par_output);
		if(par_output.messages.errors.size() > 0)
			return false;
		
		std::free(file.value());
		return true;
	}

	void test_malloc_speed_for_different_sizes()
	{
		void* block;
		
		auto start = std::chrono::high_resolution_clock::now();
		block = malloc(128);
		auto end = std::chrono::high_resolution_clock::now();
		auto duration1 = std::chrono::duration_cast<std::chrono::nanoseconds>(end - start).count();
		
		start = std::chrono::high_resolution_clock::now();
		free(block);
		end = std::chrono::high_resolution_clock::now();
		auto duration1free = std::chrono::duration_cast<std::chrono::nanoseconds>(end - start).count();
		
		
		
		
		start = std::chrono::high_resolution_clock::now();
		block = malloc(1024);
		end = std::chrono::high_resolution_clock::now();
		auto duration2 = std::chrono::duration_cast<std::chrono::nanoseconds>(end - start).count();
		
		start = std::chrono::high_resolution_clock::now();
		free(block);
		end = std::chrono::high_resolution_clock::now();
		auto duration2free = std::chrono::duration_cast<std::chrono::nanoseconds>(end - start).count();
		
		
		
		
		start = std::chrono::high_resolution_clock::now();
		block = malloc(8192);
		end = std::chrono::high_resolution_clock::now();
		auto duration3 = std::chrono::duration_cast<std::chrono::nanoseconds>(end - start).count();
		
		start = std::chrono::high_resolution_clock::now();
		free(block);
		end = std::chrono::high_resolution_clock::now();
		auto duration3free = std::chrono::duration_cast<std::chrono::nanoseconds>(end - start).count();
		
		
		
		
		start = std::chrono::high_resolution_clock::now();
		block = malloc(65536);
		end = std::chrono::high_resolution_clock::now();
		auto duration4 = std::chrono::duration_cast<std::chrono::nanoseconds>(end - start).count();
		
		start = std::chrono::high_resolution_clock::now();
		free(block);
		end = std::chrono::high_resolution_clock::now();
		auto duration4free = std::chrono::duration_cast<std::chrono::nanoseconds>(end - start).count();
		
		
		
		
		std::cout << "malloc 128 bytes: " << duration1 << " ns" << std::endl;
		std::cout << "free 128 bytes: " << duration1free << " ns" << std::endl << std::endl;
		
		std::cout << "malloc 1024 bytes: " << duration2 << " ns" << std::endl;
		std::cout << "free 1024 bytes: " << duration2free << " ns" << std::endl << std::endl;
		
		std::cout << "malloc 8192 bytes: " << duration3 << " ns" << std::endl;
		std::cout << "free 8192 bytes: " << duration3free << " ns" << std::endl << std::endl;
		
		std::cout << "malloc 65536 bytes: " << duration4 << " ns" << std::endl;
		std::cout << "free 65536 bytes: " << duration4free << " ns" << std::endl << std::endl;
	}
	
	void test_string_collection_against_vector_iteration_and_append_speed()
	{
		const u64 ITERATIONS = 4000000;

		// Test string is 128 characters long (without null-terminator)
		string_collection collection{ (128 + 1 + 8) * ITERATIONS };

		auto start = std::chrono::high_resolution_clock::now();

		for(u64 i = 0; i < ITERATIONS; i += 1)
		{
			collection.add("Hello World 375817359871935789137958891375891375897139857981375891375987138951938578913759817395981357139875913857uesarhgfuheafg", 128);
		}
		
		auto end = std::chrono::high_resolution_clock::now();
		auto duration_string_collection_append = std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count();

		std::vector<std::string> vector;
		vector.reserve(ITERATIONS);

		start = std::chrono::high_resolution_clock::now();

		for(u64 i = 0; i < ITERATIONS; i += 1)
		{
			vector.emplace_back("Hello World 375817359871935789137958891375891375897139857981375891375987138951938578913759817395981357139875913857uesarhgfuheafg");
		}
		
		end = std::chrono::high_resolution_clock::now();
		auto duration_vector_append = std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count();

		start = std::chrono::high_resolution_clock::now();

		std::string str1;
		for(u64 i = 0; i < ITERATIONS; i += 1)
		{
			char* item = collection.get(i);
			u64 length = collection.length_at(i);

			char* substring_start = static_cast<char*>(std::malloc(10));
			char* substring_end = static_cast<char*>(std::malloc(10));
			std::memcpy(substring_start, item, 9);
			std::memcpy(substring_end, (item + length - 10), 9);
			substring_start[9] = '\0';
			substring_end[9] = '\0';

			str1.append(substring_start);
			str1.append(substring_end);

			std::free(substring_start);
			std::free(substring_end);
		}
		
		end = std::chrono::high_resolution_clock::now();
		auto duration_string_collection_iterate = std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count();
		
		start = std::chrono::high_resolution_clock::now();

		std::string str2;
		for(u64 i = 0; i < ITERATIONS; i += 1)
		{
			char* item = vector[i].data();
			u64 length = vector[i].length();

			char* substring_start = static_cast<char*>(std::malloc(10));
			char* substring_end = static_cast<char*>(std::malloc(10));
			std::memcpy(substring_start, item, 9);
			std::memcpy(substring_end, (item + length - 10), 9);
			substring_start[9] = '\0';
			substring_end[9] = '\0';

			str2.append(substring_start);
			str2.append(substring_end);

			std::free(substring_start);
			std::free(substring_end);
		}
		
		end = std::chrono::high_resolution_clock::now();
		auto duration_vector_iterate = std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count();

		if (str1 != str2)
			std::cout << "Test is not working correctly" << std::endl;

		std::cout << "string_collection append: " << duration_string_collection_append << " ms" << std::endl;
		std::cout << "vector append: " << duration_vector_append << " ms" << std::endl << std::endl;

		std::cout << "string_collection iterate: " << duration_string_collection_iterate << " ms" << std::endl;
		std::cout << "vector iterate: " << duration_vector_iterate << " ms" << std::endl << std::endl;
	}
}