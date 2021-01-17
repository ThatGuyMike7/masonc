#ifndef MASONC_TEST_DEPENDENCY_LIST_HPP
#define MASONC_TEST_DEPENDENCY_LIST_HPP

#include <dependency_list.hpp>

#include <common.hpp>

#include <vector>

namespace masonc::test::dependency_list
{
    masonc::dependency_list<int> cyclic_graph();
    masonc::dependency_list<int> cyclic_graph_2();
    masonc::dependency_list<int> cyclic_graph_3();

    masonc::dependency_list<int> valid_graph();

    bool contains_cycle(const masonc::dependency_list<int>& graph, std::vector<u64> cycle);

    void test_add_vertex_and_iterator();
    void test_find();
    void test_find_cycles();
}

#endif