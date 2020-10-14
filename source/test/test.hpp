// Include this file to get access to all tests.

#ifndef $_MASONC_TEST_HPP_$
#define $_MASONC_TEST_HPP_$

namespace masonc::test
{
    void perform_all_tests();

    void perform_iterator_tests();
    void perform_dependency_graph_tests();
    void perform_parser_tests();
}

#endif