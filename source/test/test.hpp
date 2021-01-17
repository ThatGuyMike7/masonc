// Include this file to get access to all tests.

#ifndef MASONC_TEST_HPP
#define MASONC_TEST_HPP

namespace masonc::test
{
    void perform_all_tests();

    void perform_iterator_tests();
    void perform_dependency_list_tests();
    //void perform_dependency_graph_tests();
    void perform_parser_tests();
}

#endif