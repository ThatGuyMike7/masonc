#ifndef MASONC_DEPENDENCY_LIST_HPP
#define MASONC_DEPENDENCY_LIST_HPP

#include <common.hpp>
#include <type_util.hpp>
#include <iterator.hpp>

#include <vector>
#include <optional>

namespace masonc
{
    template <typename value_t>
    struct dependency_list_vertex
    {
        template <typename value_t>
        friend struct dependency_list;

        value_t value;

        dependency_list_vertex(value_t value)
            : value(value)
        { }

        bool operator== (const dependency_list_vertex<value_t>& other) const
        {
            return (value == other.value) && (adjacency_list == other.adjacency_list);
        }

    private:
        // Every vertex in "vertices" contains a list of indices that point to other vertices.
        std::vector<u64> adjacency_list;
    };

    // Directed graph implemented using adjacency list.
    template <typename value_t>
    struct dependency_list : public iterable<dependency_list_vertex<value_t>>
    {
        using vertex_t = dependency_list_vertex<value_t>;

        dependency_list()
        { }

        // If the given vertex is not adjacent to "dependency_vertex", adds that adjacency.
        // Performs one less search than the overload taking "const_ref_t<value_t>",
        // so it should be slightly faster.
        void add_adjacency(vertex_t* vertex, vertex_t* dependency_vertex)
        {
            std::optional<u64> dependency_vertex_index = find_index(dependency_vertex->value);

            assume(dependency_vertex_index.has_value());

            if (!is_adjacent(*vertex, *dependency_vertex)) {
                vertex->adjacency_list.push_back(dependency_vertex_index.value());
            }
            /*
            if (is_dependent(dependency_vertex, vertex)) {
                // Adding new dependency will result in a cycle.
            }
            else {
                // No cycle.
            }
            */
        }

        // If there is no vertex with value "value" that is adjacent to a vertex with
        // value "dependency", adds that adjacency.
        void add_adjacency(const_ref_t<value_t> value, const_ref_t<value_t> dependency)
        {
            vertex_t* vertex = find(value);
            std::optional<u64> dependency_vertex_index = find_index(dependency);

            if (vertex != nullptr && dependency_vertex_index.has_value()) {
                vertex_t* dependency_vertex = &vertices[dependency_vertex_index.value()];

                if (!is_adjacent(*vertex, *dependency_vertex)) {
                    vertex->adjacency_list.push_back(dependency_vertex_index.value());
                }
                /*
                if (is_dependent(dependency_vertex, vertex)) {
                    // Adding new dependency will result in a cycle.
                }
                else {
                    // No cycle.
                }
                */
            }
        }

        // Returns true if "a" is adjacent to "b", that means if "a" points to "b".
        bool is_adjacent(const vertex_t& a, const vertex_t& b)
        {
            for (u64 i = 0; i < a.adjacency_list.size(); i += 1) {
                u64 a_index = a.adjacency_list[i];

                if (vertices[a_index] == b)
                    return true;
            }

            return false;
        }

        // Returns true if "vertex" depends on "dependency_vertex",
        // that means if there is a path from "vertex" to "dependency_vertex".
        bool is_dependent(const vertex_t& vertex, const vertex_t& dependency_vertex)
        {
            bool result = false;
            std::vector<vertex_t*> seen;

            // Rough estimate.
            seen.reserve(16);

            is_dependent_recurse(vertex, dependency_vertex, &seen, &result);

            return result;
        }

        // If no vertex with the specified value exists yet, adds a vertex with that value
        // to the end of the list and returns a pointer to it.
        // Returns "nullptr" if a vertex with that value already exists.
        vertex_t* add_vertex(const_ref_t<value_t> value)
        {
            vertex_t* search_value = find(value);

            if (search_value == nullptr) {
                // Vertex does not exist yet.
                return &vertices.emplace_back(vertex_t{ value });
            }
            else {
                // Vertex already exists.
                return nullptr;
            }
        }

        // Returns "nullptr" if no vertex with the specified value was not found.
        vertex_t* find(const_ref_t<value_t> value)
        {
            for (u64 i = 0; i < vertices.size(); i += 1) {
                vertex_t* vertex = &vertices[i];

                if (vertex->value == value) {
                    return vertex;
                }
            }

            return nullptr;
        }

        // Returns the index of the vertex with the specified value if it exists.
        std::optional<u64> find_index(const_ref_t<value_t> value)
        {
            for (u64 i = 0; i < vertices.size(); i += 1) {
                vertex_t* vertex = &vertices[i];

                if (vertex->value == value) {
                    return std::optional<u64>{ i };
                }
            }

            return std::nullopt;
        }

        // If there are multiple sets of vertices that are not connected
        // in any way - like multiple different graphs - this function only finds
        // cycles within one of these graphs that contains the vertex at "start_index".
        void find_cycles(u64 start_index)
        {
            m_cycles.clear();

            std::vector<index_resolved_pair> seen;

            // Rough estimate.
            seen.reserve(16);

            seen.emplace_back(index_resolved_pair{ start_index, false });
            find_cycles_recurse(&seen[0], &seen);
        }

        const std::vector<std::vector<u64>>& cycles() const
        {
            return m_cycles;
        }

        iterator<vertex_t> begin() override
        {
            return iterator<vertex_t>{ vertices.data() };
        }

        iterator<vertex_t> end() override
        {
            return iterator<vertex_t>{ vertices.data() + vertices.size() };
        }

    private:
        // Vertices are not stored in any particular order.
        // New ones are pushed back so that indices remain stable.
        std::vector<vertex_t> vertices;

        std::vector<std::vector<u64>> m_cycles;

        // Returns vertices stored in depth-first post-order, so that simply iterating over them
        // allows resolving dependencies in the correct order, though only if there are no cycles.
        std::vector<vertex_t> order()
        {
            // TODO: Implementation.
            return vertices;
        }

        void is_dependent_recurse(const vertex_t& vertex, const vertex_t& dependency_vertex,
            std::vector<vertex_t*>* seen, bool* result)
        {
            for (u64 i = 0; i < vertex.adjacency_list.size(); i += 1) {
                u64 vertex_index = vertex.adjacency_list[i];
                vertex_t* current_vertex = &vertices[vertex_index];

                if (current_vertex->value == dependency_vertex.value) {
                    *result = true;
                    return;
                }

                for (u64 j = 0; j < seen->size(); j += 1) {
                    vertex_t* seen_vertex = (*seen)[j];

                    if (seen_vertex == current_vertex) {
                        // The same pointer has been seen more than once,
                        // ignore it since it's a cycle and we don't want to blow up the stack.
                        goto CONTINUE;
                    }
                }

                seen->push_back(current_vertex);
                is_dependent_recurse(*current_vertex, dependency_vertex, seen, result);

                CONTINUE:
            }
        }

        struct index_resolved_pair
        {
            u64 index;
            bool resolved;
        };

        void find_cycles_recurse(index_resolved_pair* vertex_index_resolved,
            std::vector<index_resolved_pair>* seen)
        {
            u64 vertex_index = vertex_index_resolved->index;
            vertex_t* vertex = &vertices[vertex_index];

            // Loop over the vertex' adjacent vertices.
            for (u64 i = 0; i < vertex->adjacency_list.size(); i += 1) {
                u64 adjacent_vertex_index = vertex->adjacency_list[i];
                vertex_t* adjacent_vertex = &vertices[adjacent_vertex_index];

                // Loop over the vertices that have been seen before.
                for (u64 j = 0; j < seen->size(); j += 1) {
                    u64 seen_vertex_index = (*seen)[j].index;
                    vertex_t* seen_vertex = &vertices[seen_vertex_index];
                    bool seen_vertex_resolved = (*seen)[j].resolved;

                    // Do we have a path that has already been taken?
                    if (seen_vertex == adjacent_vertex) {

                        // Do we have a cycle?
                        if (!seen_vertex_resolved) {
                            std::vector<u64> cycle;
                            cycle.reserve(seen->size() - 1 - j);

                            // We can "backtrack" to find out which vertices are part of
                            // the cycle.
                            for (u64 k = j; k < seen->size(); k += 1) {
                                cycle.push_back((*seen)[k].index);
                            }

                            // FIXME: Maybe check if this cycle already exists
                            //        and don't add it in that case?
                            m_cycles.push_back(cycle);
                        }

                        // Do not go the same path again.
                        goto CONTINUE;
                    }
                }

                seen->push_back(index_resolved_pair{ adjacent_vertex_index, false });
                find_cycles_recurse(&seen->back(), seen);

                CONTINUE:
                ;
            }

            vertex_index_resolved->resolved = true;
        }
    };
}

#endif