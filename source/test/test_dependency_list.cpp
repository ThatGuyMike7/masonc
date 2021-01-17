#include <test_dependency_list.hpp>

#include <common.hpp>

#include <stdexcept>
#include <iostream>

namespace masonc::test::dependency_list
{
    masonc::dependency_list<int> cyclic_graph()
    {
        masonc::dependency_list<int> graph;

        graph.add_vertex(0);
        graph.add_vertex(1);
        graph.add_vertex(2);
        graph.add_vertex(3);

        graph.add_adjacency(0, 1);
        graph.add_adjacency(1, 2);
        graph.add_adjacency(2, 3);
        graph.add_adjacency(3, 0); // Cycle (rectangle).

        graph.add_adjacency(0, 2); // Cycle (diagonal).

        return graph;
    }

    masonc::dependency_list<int> cyclic_graph_2()
    {
        masonc::dependency_list<int> graph;

        graph.add_vertex(0);
        graph.add_vertex(1);

        graph.add_adjacency(0, 1);
        graph.add_adjacency(1, 0); // Cycle.

        return graph;
    }

    masonc::dependency_list<int> cyclic_graph_3()
    {
        masonc::dependency_list<int> graph;

        graph.add_vertex(0);
        graph.add_vertex(1);
        graph.add_vertex(2);
        graph.add_vertex(3);
        graph.add_vertex(4);

        graph.add_adjacency(0, 1);
        graph.add_adjacency(0, 2);

        graph.add_adjacency(1, 3);
        graph.add_adjacency(2, 3);

        graph.add_adjacency(3, 4);
        graph.add_adjacency(4, 1); // Cycle.
        graph.add_adjacency(4, 2); // Cycle.

        return graph;
    }

    masonc::dependency_list<int> valid_graph()
    {
        masonc::dependency_list<int> graph;

        graph.add_vertex(0);
        graph.add_vertex(1);
        graph.add_vertex(2);
        graph.add_vertex(3);
        graph.add_vertex(4);
        graph.add_vertex(5);
        graph.add_vertex(6);

        graph.add_adjacency(0, 1);
        graph.add_adjacency(1, 2);

        graph.add_adjacency(2, 3);
        graph.add_adjacency(2, 4);

        graph.add_adjacency(4, 5);

        graph.add_adjacency(0, 6);
        graph.add_adjacency(6, 2);

        return graph;
    }

    // TODO: Perhaps write a test for this function.
    bool contains_cycle(const masonc::dependency_list<int>& graph, std::vector<u64> cycle)
    {
        const auto& cycles = graph.cycles();

        for (u64 i = 0; i < cycles.size(); i += 1) {
            const std::vector<u64>& current_cycle = cycles[i];

            if (cycle.size() != current_cycle.size())
                continue;

            // Check if each element in "cycle" exists somewhere in "current_cycle".
            u64 cnt = 0;
            for (u64 j = 0; j < cycle.size(); j += 1) {
                u64 current_element = cycle[j];

                for (u64 k = 0; k < current_cycle.size(); k += 1) {
                    if (current_cycle[k] == current_element) {
                        cnt += 1;
                        break;
                    }
                }
            }

            if (cnt == cycle.size())
                return true;
        }

        return false;
    }

    void test_add_vertex_and_iterator()
    {
        masonc::dependency_list<int> graph;

        graph.add_vertex(0);
        graph.add_vertex(1);
        graph.add_vertex(2);

        if ((*graph.begin()).value != 0 ||
            (*(graph.begin() + 1)).value != 1 ||
            (*(graph.end() - 1)).value != 2)
        {
            throw std::runtime_error{ "dependency_list add vertex and iterator test failed" };
        }
    }

    void test_find()
    {
        masonc::dependency_list<int> graph;
        graph.add_vertex(0);
        graph.add_vertex(1);
        graph.add_vertex(2);

        if (graph.find(0) == nullptr || graph.find(1) == nullptr || graph.find(2) == nullptr) {
            throw std::runtime_error{ "dependency_list add vertex and iterator test failed" };
        }
    }

    void test_find_cycles()
    {
        // Graph 1
        masonc::dependency_list<int> graph = cyclic_graph();
        std::optional<u64> root_index = graph.find_index(0);

        graph.find_cycles(root_index.value());

        if (!contains_cycle(graph, { 0, 1, 2, 3 })) {
            throw std::runtime_error{ "dependency_list find cycles test failed" };
        }

        // Graph 2
        graph = cyclic_graph_2();
        root_index = graph.find_index(0);

        graph.find_cycles(root_index.value());

        if (!contains_cycle(graph, { 0, 1 })) {
            throw std::runtime_error{ "dependency_list find cycles test failed" };
        }

        // Graph 3
        graph = cyclic_graph_3();
        root_index = graph.find_index(0);

        graph.find_cycles(root_index.value());

        if (!contains_cycle(graph, { 1, 3, 4 }) ||
            !contains_cycle(graph, { 2, 3, 4 }))
        {
            throw std::runtime_error{ "dependency_list find cycles test failed" };
        }

        // Valid graph
        graph = valid_graph();
        root_index = graph.find_index(0);

        graph.find_cycles(root_index.value());

        if (graph.cycles().size() > 0) {
            throw std::runtime_error{ "dependency_list find cycles test failed" };
        }

        /*
        for (u64 i = 0; i < graph.cycles().size(); i += 1) {
            auto cycle = graph.cycles()[i];

            for (u64 j = 0; j < cycle.size(); j += 1) {
                std::cout << cycle[j] << " ";
            }

            std::cout << '\n';
        }

        std::cout << std::flush;
        */
    }
}