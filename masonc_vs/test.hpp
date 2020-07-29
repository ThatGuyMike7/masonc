#ifndef $_MASONC_TEST_HPP_$
#define $_MASONC_TEST_HPP_$

#include <vector>
#include <string>

namespace masonc
{
    struct test_parse_in_directory_output
    {
        std::vector<std::string> files;
        std::vector<bool> matched_expected;
    };
    
    // Returns a list of file paths and booleans specifying if test-parsing them
    // matches the "expected" value.
    // 
    // All files in "directory_path" and its sub-directories are tested individually.
    test_parse_in_directory_output test_parse_in_directory(const char* directory_path, bool expected);
    
	// Returns true if lexing and parsing was successful and
    // returns false if any errors occured.
	bool test_parse(const char* filename);

	void test_malloc_speed_for_different_sizes();
    void test_string_collection_against_vector_iteration_and_append_speed();
}

#endif