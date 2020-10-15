#ifndef $_MASONC_DEPENDENCY_GRAPH_HPP_$
#define $_MASONC_DEPENDENCY_GRAPH_HPP_$

#include <common.hpp>
#include <type_util.hpp>

#include <functional>
#include <algorithm>
#include <type_traits>
#include <vector>
#include <optional>

namespace masonc
{
    template <typename value_t>
    struct dependency_graph_no_resolver
    {
        using const_value_ref_t = const_ref_t<value_t>;
        void operator() (const_value_ref_t value) { }
    };

    template <typename value_t,
              typename value_comparator_less_t = std::less<value_t>,
              typename resolver_t = dependency_graph_no_resolver<value_t>>
    struct dependency_graph_vertex
    {
    public:
        using value_ref_t = ref_t<value_t>;
        using const_value_ref_t = const_ref_t<value_t>;

    protected:
        value_t m_value;
        value_comparator_less_t value_comparator_less;

        std::vector<dependency_graph_vertex> m_edges;

        resolver_t m_resolver;

    public:
        dependency_graph_vertex(const_value_ref_t value)
            : m_value(value)
        { }

        bool operator< (const dependency_graph_vertex& other) const
        {
            return value_comparator_less(m_value, other.m_value);
        }

        const_value_ref_t value() const
        {
            return m_value;
        }

        const std::vector<dependency_graph_vertex>& edges() const
        {
            return m_edges;
        }

        const resolver_t& resolver() const
        {
            return m_resolver;
        }

        dependency_graph_vertex* at(u64 i)
        {
            return &m_edges[i];
        }

        // Insert a new direct dependency and return a pointer to it.
        // May invalidate other pointers, references and iterators.
        dependency_graph_vertex* insert(const dependency_graph_vertex& value)
        {
            auto insert_vertex = m_edges.insert(
                std::upper_bound(m_edges.begin(), m_edges.end(), value),
                value
            );

            return &(*insert_vertex);
        }

        // Insert a new direct dependency and return a pointer to it.
        // May invalidate other pointers, references and iterators.
        dependency_graph_vertex* insert(const_value_ref_t value)
        {
            return insert(dependency_graph_vertex{ value });
        }

        // Find and return the first direct dependency with a specific "value",
        // i.e. search in all directly connected vertices.
        std::optional<const dependency_graph_vertex*> find_direct(const_value_ref_t value)
        {
            dependency_graph_vertex value_vertex{ value };

            auto search_element = std::lower_bound(
                m_edges.begin(), m_edges.end(), value_vertex
            );

            if(search_element != m_edges.end() && search_element->value() == value)
                return &(*search_element);
            else
                return std::nullopt;
        }

        // Find the first direct or indirect dependency with a specific "value".
        // Check if there are circular dependencies first to avoid a crash due to infinite looping.
        std::optional<const dependency_graph_vertex*> find(const_value_ref_t value)
        {
            auto direct_result = find_direct(value);
            if (direct_result)
                return direct_result;

            for (u64 i = 0; i < m_edges.size(); i += 1) {
                auto indirect_result = m_edges[i].find(value);
                if (indirect_result)
                    return indirect_result;
            }

            return std::nullopt;
        }

        bool resolve()
        {
            // Which values were seen before through iteration
            // (only relevant to "resolve_recurse").
            std::vector<value_ref_t> seen;

            // Which of these values that were seen before have been resolved
            // (only relevant to "resolve_recurse").
            std::vector<BOOL> seen_resolved;

            // Whether or not the tree is acyclic.
            bool acyclic = true;

            resolve_recurse(this, &seen, &seen_resolved, &acyclic);

            return acyclic;
        }

    protected:
        void resolve_recurse_edges(std::vector<dependency_graph_vertex>* edges,
                                   std::vector<value_ref_t>* seen, std::vector<BOOL>* seen_resolved,
                                   bool* acyclic)
        {
            for (u64 i = 0; i < edges->size(); i += 1) {
                resolve_recurse(&(*edges)[i], seen, seen_resolved, acyclic);
            }
        }

        // Perform depth-first post-order traversal, invoking the resolver on every vertex.
        void resolve_recurse(dependency_graph_vertex* current_vertex,
                             std::vector<value_ref_t>* seen, std::vector<BOOL>* seen_resolved,
                             bool* acyclic)
        {
            value_ref_t current_value = current_vertex->m_value;
            auto search_value = std::lower_bound(seen->begin(), seen->end(), current_value);

            if (search_value != seen->end() && *search_value != current_value ||
                search_value == seen->end())
            {
                // The value has not been seen yet, add it to "seen" and
                // a corresponding "FALSE" value to "seen_resolved".
                auto seen_insertion_point = std::upper_bound(seen->begin(), seen->end(), current_value);

                auto seen_resolved_insertion_point = seen_resolved->begin();
                std::advance(seen_resolved_insertion_point, seen_insertion_point - seen->begin());

                seen->insert(seen_insertion_point, current_value);
                seen_resolved->insert(seen_resolved_insertion_point, FALSE);

                // Traverse the tree.
                resolve_recurse_edges(&current_vertex->m_edges, seen, seen_resolved, acyclic);

                // Access the data.
                m_resolver(current_value);

                // Mark it as resolved.
                // For that, we need to find it again because insertions to vectors
                // (specifically "seen" and "seen_resolved") invalidate iterators and indices.
                //
                // Since "seen" contains unique values and their indices correspond
                // to values in "seen_resolved" with the same indices, we can use "seen" to
                // find the correct index in "seen_resolved".
                search_value = std::lower_bound(seen->begin(), seen->end(), current_value);

                auto is_resolved_point = seen_resolved->begin();
                std::advance(is_resolved_point, search_value - seen->begin());

                *is_resolved_point = TRUE;
            }
            else {
                // The value has been seen before.
                //
                // If it and its dependencies are not resolved yet,
                // we have found a circular dependency.
                auto is_resolved_point = seen_resolved->begin();
                std::advance(is_resolved_point, search_value - seen->begin());

                if (*is_resolved_point == FALSE) {
                    *acyclic = false;
                    return;
                }
            }
        }
    };

    // Rooted directed graph that can be verified to be acyclic.
    // Each value in every vertex must be unique, unless they refer to the same vertex.
    // "resolver_t" must implement "operator()" taking "const_value_ref_t" as its only parameter.
    template <typename value_t,
              typename value_comparator_less_t = std::less<value_t>,
              typename resolver_t = dependency_graph_no_resolver<value_t>>
    struct dependency_graph
    {
    public:
        using value_ref_t = ref_t<value_t>;
        using const_value_ref_t = const_ref_t<value_t>;

        using vertex_t = dependency_graph_vertex<value_t, value_comparator_less_t, resolver_t>;

        dependency_graph(const_value_ref_t root_value)
            : root(vertex_t{ root_value })
        { }

        vertex_t root;
    };
}

#endif