#ifndef $_MASONC_TEST_HPP_$
#define $_MASONC_TEST_HPP_$

namespace masonc
{
	// Returns false if any lexer/par errors occured
	//bool test_parser(const char* filename);

	void test_malloc_speed_for_different_sizes();
    void test_string_collection_against_vector_iteration_and_append_speed();
}

#endif