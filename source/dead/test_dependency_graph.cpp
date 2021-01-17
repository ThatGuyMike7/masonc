#include <test_dependency_graph.hpp>

#include <dependency_graph.hpp>

#include <cstdlib>
#include <stdexcept>

namespace masonc::test::dependency_graph
{
    void test_insert()
    {
        auto graph = get_test_graph();

        if (graph.root.at(0)->value() != 1 ||
            graph.root.at(1)->value() != 2 ||
            graph.root.at(2)->value() != 3 ||

            graph.root.at(0)->at(0)->value() != 11 ||
            graph.root.at(1)->at(0)->value() != 21 ||
            graph.root.at(1)->at(1)->value() != 22 ||
            graph.root.at(2)->at(0)->value() != 31 ||

            graph.root.at(0)->at(1)->value() != 1111 ||
            graph.root.at(1)->at(1)->at(0)->value() != 1111)
        {
            throw std::runtime_error{ "dependency graph insert test failed" };
            std::exit(-1);
        }
    }

    void test_find_direct()
    {
        auto graph = get_test_graph();

        auto find_result_11 = graph.root.at(0)->find_direct(11);
        auto find_result_1111 = graph.root.at(0)->find_direct(1111);

        if (!find_result_11 || find_result_11.value()->value() != 11 ||
            !find_result_1111 || find_result_1111.value()->value() != 1111)
        {
            throw std::runtime_error{ "dependency graph find direct test failed" };
            std::exit(-1);
        }
    }

    void test_find()
    {

    }

    void test_is_circular()
    {
        auto non_circular_graph = get_test_graph<dependency_graph_print_resolver<int>>();
        auto circular_graph = get_circular_test_graph<dependency_graph_print_resolver<int>>();

        if (!non_circular_graph.root.resolve()) {
            throw std::runtime_error{ "dependency graph is circular test failed" };
            std::exit(-1);
        }

        std::cout << std::endl;

        if (circular_graph.root.resolve()) {
            throw std::runtime_error{ "dependency graph is circular test failed" };
            std::exit(-1);
        }

    }
}