#include <test.hpp>

#include <test_iterator.hpp>
#include <test_dependency_graph.hpp>
#include <test_parser.hpp>
#include <test_misc.hpp>

#include <common.hpp>
#include <log.hpp>

#include <cstdlib>
#include <string>
#include <chrono>
#include <iostream>
#include <optional>

namespace masonc::test
{
    void perform_all_tests()
    {
        perform_iterator_tests();
        perform_dependency_graph_tests();
        perform_parser_tests();
    }

    void perform_iterator_tests()
    {
        masonc::test::iterator::test_forward_iteration();
        masonc::test::iterator::test_reverse_iteration();
        masonc::test::iterator::test_STL_algorithms();
        masonc::test::iterator::test_iterator_data_iteration();
    }

    void perform_dependency_graph_tests()
    {
        masonc::test::dependency_graph::test_insert();
        masonc::test::dependency_graph::test_find_direct();
        masonc::test::dependency_graph::test_is_circular();
    }

    void perform_parser_tests()
    {
        auto parse_tests_pass = masonc::test::parser::test_parse_in_directory("tests/pass", true);
        for(u64 i = 0; i < parse_tests_pass.matched_expected.size(); i += 1) {
            if(!parse_tests_pass.matched_expected[i]) {
                log_error(std::string{
                    "parse test failed (expected success): " + parse_tests_pass.files[i]
                }.c_str());

                parse_tests_pass.message_lists[i].print_errors();
            }
        }

        auto parse_tests_fail = masonc::test::parser::test_parse_in_directory("tests/fail", false);
        for(u64 i = 0; i < parse_tests_fail.matched_expected.size(); i += 1) {
            if(!parse_tests_fail.matched_expected[i]) {
                log_error(std::string{
                    "parse test succeeded (expected failure): " + parse_tests_fail.files[i]
                }.c_str());
            }
        }
    }
}