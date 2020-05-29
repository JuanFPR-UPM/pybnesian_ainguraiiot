#ifndef PGM_DATASET_DAG_HPP
#define PGM_DATASET_DAG_HPP

#include <iostream>
#include <pybind11/pybind11.h>
#include <boost/graph/adjacency_matrix.hpp>
#include <boost/graph/adjacency_list.hpp>
#include <boost/graph/depth_first_search.hpp>


namespace py = pybind11;
using boost::adjacency_matrix, boost::adjacency_list, boost::directedS, boost::setS, boost::vecS, boost::property, boost::vertex_index_t,
    boost::property_map, boost::vertex_index;

using adj_m = adjacency_matrix<directedS, property<vertex_index_t, int>>;
using adj_l = adjacency_list<setS, vecS, directedS, property<vertex_index_t, int>>;

namespace graph {

    using arc_vector = std::vector<std::pair<std::string, std::string>>;

    template<typename Graph>
    class Dag {
    public:
        using node_descriptor = typename Graph::vertex_descriptor;
        using edge_descriptor = typename Graph::edge_descriptor;

        using node_iterator_t = typename boost::graph_traits<Graph>::vertex_iterator;
        using edge_iterator_t = typename boost::graph_traits<Graph>::edge_iterator;
        using in_edge_iterator_t = typename boost::graph_traits<Graph>::in_edge_iterator;

        using nodes_size_type = typename boost::graph_traits<Graph>::vertices_size_type;
        using edges_size_type = typename boost::graph_traits<Graph>::edges_size_type;
        using degree_size_type = typename boost::graph_traits<Graph>::degree_size_type;

        template<typename = std::enable_if_t<std::is_default_constructible_v<Graph>>>
        Dag() = delete;

        Dag(int nnodes) : g(nnodes) { };

        // TODO Implement adding arcs.
        Dag(int nnodes, const arc_vector& arcs) : g(nnodes) {};

        nodes_size_type num_nodes() const {
            return num_vertices(g);
        }

        edges_size_type num_arcs() const {
            return num_edges(g);
        }

        degree_size_type num_parents(node_descriptor node) const {
            return boost::in_degree(node, g);
        }

        degree_size_type num_children(node_descriptor node) const {
            return boost::out_degree(node, g);
        }

        std::pair<node_iterator_t, node_iterator_t> nodes() const {
            return boost::vertices(g);
        }

        std::pair<edge_iterator_t, edge_iterator_t> edges() const {
            return boost::edges(g);
        }

        std::pair<in_edge_iterator_t, in_edge_iterator_t> get_parent_edges(node_descriptor node) const {
            return in_edges(node, g);
        }

        int index(node_descriptor n) const {
            return get(boost::vertex_index, g)[n];
        }

        node_descriptor node(nodes_size_type index) const {
            return vertex(index, g);
        }

        node_descriptor source(edge_descriptor edge) const {
            return boost::source(edge, g);
        }

        node_descriptor target(edge_descriptor edge) const {
            return boost::target(edge, g);
        }

        bool has_edge(node_descriptor source, node_descriptor dest) const {
            return boost::edge(source, dest, g).second;
        }

        void add_edge(node_descriptor source, node_descriptor dest) {
            boost::add_edge(source, dest, g);
        }

        class dummy_visitor : public boost::dfs_visitor<> {};

        bool has_path(node_descriptor source, node_descriptor dest) const {
            bool path = false;

            std::vector<boost::default_color_type> vertex_color(num_vertices(g));
            auto idmap = get(vertex_index, g);
            auto colors = make_iterator_property_map(vertex_color.begin(), idmap);
        
            boost::depth_first_visit(g, source, dummy_visitor(), colors, 
                [&path, &dest](auto node, auto ) {
                    if (node == dest) {
                        path = true;
                        return true;
                    } else {
                        return false;
                    }
                });

            return path;
        }

        void remove_edge(node_descriptor source, node_descriptor dest) {
            boost::remove_edge(source, dest, g);
        }

    private:
        Graph g;
    };

    using AdjMatrixDag = Dag<adj_m>;
    using AdjListDag = Dag<adj_l>;
}



#endif //PGM_DATASET_DAG_HPP