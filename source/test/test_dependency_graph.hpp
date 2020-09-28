#ifndef $_MASONC_TEST_DEPENDENCY_GRAPH_HPP_$
#define $_MASONC_TEST_DEPENDENCY_GRAPH_HPP_$

#include <dependency_graph.hpp>
#include <type_util.hpp>

#include <functional>
#include <iostream>

namespace masonc::test::dependency_graph
{
    template <typename value_t>
    struct dependency_graph_print_resolver
    {
        using value_ref_t = ref_t<value_t>;
        void operator() (const value_ref_t value)
        {
            std::cout << value << std::endl;
        }
    };

    template <typename resolver_t = dependency_graph_no_resolver<int>>
    masonc::dependency_graph<int, std::less<int>, resolver_t> get_test_graph()
    {
        masonc::dependency_graph<int, std::less<int>, resolver_t> graph{ 0 };

        graph.root.insert(1);
        graph.root.insert(2);
        graph.root.insert(3);

        graph.root.at(0)->insert(11);
        graph.root.at(1)->insert(21);
        graph.root.at(1)->insert(22);
        graph.root.at(2)->insert(31);

        // Two vertices depending on another vertex.
        graph.root.at(0)->insert(1111);
        graph.root.at(1)->at(1)->insert(1111);

        return graph;
    }

    template <typename resolver_t = dependency_graph_no_resolver<int>>
    masonc::dependency_graph<int, std::less<int>, resolver_t> get_circular_test_graph()
    {
        auto graph = get_test_graph<resolver_t>();

        // Circular dependency including the following values: 3 -> 9999 -> 1 -> 3
        graph.root.at(2)->insert(9999);
        graph.root.at(2)->at(0)->insert(1);
        graph.root.at(0)->insert(3);

        // Circular dependency including the root: 0 -> 2 -> 0
        graph.root.at(1)->insert(0);

        return graph;
    }

    // Insert vertices into a new dependency graph and then
    // test if they exist in the correct order.
    //
    // Quits the program on failure.
    void test_insert();

    // Insert vertices into a new dependency graph and then
    // test if "dependency_graph_vertex::find_direct" returns true for
    // direct dependencies and false for non-direct dependencies.
    //
    // Quits the program on failure.
    void test_find_direct();

    // Insert vertices into a new dependency graph and then
    // test if "dependency_graph_vertex::find" returns true when the value
    // is present in the graph starting at a specific vertex, and
    // false when the value is not present starting at a specific vertex.
    //
    // Quits the program on failure.
    void test_find();

    // Insert vertices into a new circular dependency graph,
    // attempt to resolve it and check if a circular dependency was detected.
    // Then do the same with a non-circular graph and test if no circular dependency was detected.
    //
    // Quits the program on failure.
    void test_is_circular();
}

#endif