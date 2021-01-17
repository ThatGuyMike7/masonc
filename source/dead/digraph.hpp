#ifndef $_MASONC_DIGRAPH_HPP_$
#define $_MASONC_DIGRAPH_HPP_$

#include <common.hpp>
#include <type_util.hpp>

#include <robin_hood.hpp>

#include <type_traits>
#include <vector>
#include <optional>
#include <algorithm>

namespace masonc
{
    template <typename vertex_t,
              typename resolver_t>
    struct digraph_basic
    {
    public:
        using vertex_ptr_t = ptr_t<vertex_t>;
        using vertex_ref_t = ref_t<vertex_t>;
        using const_vertex_ref_t = const_ref_t<vertex_t>;

        // Note: Always use "dereference<T>(x)" function if
        //       a template parameter could be a pointer or non-pointer type.

        // Returns a pointer to the vertex if it exists in the graph.
        virtual std::optional<vertex_ptr_t> find(vertex_const_ref_t vertex) = 0;

        // Returns true if "a" is linked to "b" (not commutative).
        virtual bool linked(vertex_const_ref_t a, vertex_const_ref_t b) = 0;

        // Adds a vertex to the graph without links and returns a pointer to it.
        virtual vertex_ptr_t insert(vertex_const_ref_t vertex) = 0;
        virtual vertex_ptr_t insert(vertex_t&& vertex) = 0;

        // Calls "resolver_t()" on every vertex in reverse order,
        // i.e. the leaves are invoked first, then their parents and so on until the roots.
        virtual void resolve() = 0;
    };

    // Fast search of vertices and their links.
    // Slow traversal.
    template <typename value_t,
              typename resolver_t> // :=
    struct digraph_hashmap final : public digraph_basic<value_t, resolver_t>
    {
    private:
        // Vertices associated with their connected vertices.
        robin_hood::unordered_map<vertex_t, std::vector<vertex_ptr_t>> graph;

    public:
        std::optional<vertex_ptr_t> find(vertex_const_ref_t vertex) override
        {
            auto search = graph.find(vertex);
            if (search != graph.end())
                return std::optional<vertex_ptr_t>{ &(*search) };
            else
                return std::nullopt;
        }

        bool linked(vertex_const_ref_t a, vertex_const_ref_t b) override
        {

        }
    };

    // Slow search of vertices and their links.
    // Fast traversal.
    template <typename value_t,
              typename resolver_t>
    struct digraph_adjacency_list final : public digraph_basic<value_t, resolver_t>
    {
    private:
        std::vector<vertex_t> vertices;

        // Each vertex has a list of indices of other vertices it is linked to.
        std::vector<std::vector<u64>> adjacency_list;

    public:
        std::optional<vertex_ptr_t> find(vertex_const_ref_t vertex) override
        {
            for (u64 i = 0; i < vertices.size(); i += 1) {
                if (dereference(vertices[i]) == dereference(vertex))
                    return std::optional<vertex_ptr_t>{ &dereference(vertices[i]) };
            }

            return std::nullopt;
        }

        bool linked(vertex_const_ref_t a, vertex_const_ref_t b) override
        {
            for (u64 i = 0; i < vertices.size(); i += 1) {
                if (vertices[i] == dereference(a)) {
                    std::vector<u64>& linked_vertices = adjacency_list[i];

                    for (u64 j = 0; j < linked_vertices.size(); i += 1) {
                        if (linked_vertices[j] == dereference(b))
                            return true;
                    }

                    // Vertex "a" is not linked to vertex "b".
                    return false;
                }
            }

            // Vertex "a" does not exist in the graph.
            return false;
        }

        vertex_ptr_t insert(vertex_const_ref_t vertex) override
        {
            auto inserted_vertex = dereference(vertices.emplace_back(dereference(vertex)));
            inserted_vertex.index = vertices.size() - 1;
        }

        vertex_ptr_t insert(vertex_t&& vertex) override
        {
            auto inserted_vertex = dereference(vertices.emplace_back((vertex)));
            inserted_vertex.index = vertices.size() - 1;
        }

        void resolve() override
        { }
    };

    struct digraph_adjacency_matrix final
    {

    };
}

#endif