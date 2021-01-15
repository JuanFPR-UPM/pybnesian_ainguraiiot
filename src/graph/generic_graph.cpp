#include <graph/generic_graph.hpp>
#include <util/vector.hpp>

namespace graph {

    PartiallyDirectedGraph PartiallyDirectedGraph::CompleteUndirected(const std::vector<std::string>& nodes) {
        PartiallyDirectedGraph pdag(nodes);

        for (int i = 0, limit = nodes.size() - 1; i < limit; ++i) {
            for (int j = i + 1, size = nodes.size(); j < size; ++j) {
                pdag.add_edge_unsafe(i, j);
            }
        }

        return pdag;
    }

    ConditionalPartiallyDirectedGraph 
    ConditionalPartiallyDirectedGraph::CompleteUndirected(const std::vector<std::string>& nodes,
                                                          const std::vector<std::string>& interface_nodes) {
        ConditionalPartiallyDirectedGraph cpdag(nodes, interface_nodes);

        for (int i = 0, limit = nodes.size() - 1; i < limit; ++i) {
            auto node_index = cpdag.index(nodes[i]);
            for (int j = i + 1, size = nodes.size(); j < size; ++j) {
                auto other_node_index = cpdag.index(nodes[j]);
                cpdag.add_edge_unsafe(node_index, other_node_index);
            }
        }

        for (const auto& node : nodes) {
            auto node_index = cpdag.index(node);
            for(const auto& inode : interface_nodes) {
                auto inode_index = cpdag.index(inode);
                cpdag.add_edge_unsafe(node_index, inode_index);
            }
        }

        return cpdag;
    }

    ArcStringVector ConditionalPartiallyDirectedGraph::compelled_arcs() const {
        ArcStringVector res;

        for (const auto& edge : edge_indices()) {
            if (is_interface(edge.first))
                res.push_back({name(edge.first), name(edge.second)});
            else if (is_interface(edge.second))
                res.push_back({name(edge.second), name(edge.first)});
        }

        return res;
    }

    UndirectedGraph UndirectedGraph::Complete(const std::vector<std::string>& nodes) {
        UndirectedGraph un(nodes);

        for (int i = 0, limit = nodes.size() - 1; i < limit; ++i) {
            for (int j = i + 1, size = nodes.size(); j < size; ++j) {
                un.add_edge_unsafe(i, j);
            }
        }

        return un;
    }

    ConditionalUndirectedGraph ConditionalUndirectedGraph::Complete(const std::vector<std::string>& nodes,
                                                                    const std::vector<std::string>& interface_nodes) {
        ConditionalUndirectedGraph un(nodes, interface_nodes);

        for (int i = 0, limit = nodes.size() - 1; i < limit; ++i) {
            auto node_index = un.index(nodes[i]);
            for (int j = i + 1, size = nodes.size(); j < size; ++j) {
                auto other_node_index = un.index(nodes[j]);
                un.add_edge_unsafe(node_index, other_node_index);
            }
        }

        for (const auto& node : nodes) {
            auto node_index = un.index(node);
            for(const auto& inode : interface_nodes) {
                auto inode_index = un.index(inode);
                un.add_edge_unsafe(node_index, inode_index);
            }
        }

        return un;
    }
    
    py::object load_graph(const std::string& name) {
        auto open = py::module::import("io").attr("open");
        auto file = open(name, "rb");
        auto graph = py::module::import("pickle").attr("load")(file);
        file.attr("close")();
        return graph;
    }
}
