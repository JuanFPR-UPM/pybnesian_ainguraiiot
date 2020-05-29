#ifndef PGM_DATASET_BAYESIANNETWORK_HPP
#define PGM_DATASET_BAYESIANNETWORK_HPP

#include <iterator>
#include <dataset/dataset.hpp>
#include <graph/dag.hpp>

using dataset::DataFrame;
using graph::AdjMatrixDag, graph::AdjListDag;
using boost::source;

using graph::arc_vector; 
namespace models {


    enum BayesianNetworkType {
        GAUSSIAN_NETWORK
    };


    // template<typename it> node_iterator(it b, it e) -> node_iterator<typename std::iterator_traits<Iterator>::value_type>;

    template<BayesianNetworkType T, typename DagType = AdjMatrixDag>
    class BayesianNetwork {
    
    public:
        using node_descriptor = typename DagType::node_descriptor;
        using edge_descriptor = typename DagType::edge_descriptor;

        using node_iterator_t = typename DagType::node_iterator_t;

        using nodes_size_type = typename DagType::nodes_size_type;
        using edges_size_type = typename DagType::edges_size_type;
        using degree_size_type = typename DagType::degree_size_type;

        BayesianNetwork(const std::vector<std::string>& nodes);
        BayesianNetwork(const std::vector<std::string>& nodes, const arc_vector& arcs);

        static void requires(const DataFrame& df);

        nodes_size_type num_nodes() const {
            return g.num_nodes();
        }

        edges_size_type num_edges() const {
            return g.num_edges();
        }

        const std::vector<std::string>& nodes() const {
            return m_nodes;
        }

        const std::unordered_map<std::string, int>& indices() const {
            return m_indices;
        }

        node_descriptor node(int node_index) const {
            return g.node(node_index);
        }

        node_descriptor node(const std::string& node) const {
            return g.node(m_indices.at(node));
        }

        const std::string& name(int node_index) const {
            return m_nodes[node_index];
        }

        const std::string& name(node_descriptor node) const {
            return name(g.index(node));
        }

        degree_size_type num_parents(node_descriptor node) const {
            return g.num_parents(node);
        }

        degree_size_type num_parents(int node_index) const {
            return num_parents(g.node(node_index));
        }

        degree_size_type num_parents(const std::string& node) const {
            return num_parents(m_indices.at(node));
        }

        degree_size_type num_children(node_descriptor node) const {
            return g.num_children(node);
        }

        degree_size_type num_children(int node_index) const {
            return num_children(g.node(node_index));
        }

        degree_size_type num_children(const std::string& node) const {
            return num_children(m_indices.at(node));
        }

        int index(node_descriptor n) const {
            return g.index(n);
        }

        std::vector<std::reference_wrapper<const std::string>> get_parents(node_descriptor node) const {
            std::vector<std::reference_wrapper<const std::string>> parents;
            auto it_parents = g.get_parent_edges(node);

            for (auto it = it_parents.first; it != it_parents.second; ++it) {
                auto parent = g.source(*it);
                auto parent_index = g.index(parent);
                parents.push_back(m_nodes[parent_index]);
            }

            return parents;
        }

        std::vector<std::reference_wrapper<const std::string>> get_parents(int node_index) const {
            return get_parents(g.node(node_index));
        }

        std::vector<std::reference_wrapper<const std::string>> get_parents(const std::string& node) const {
            return get_parents(m_indices.at(node));
        }

        std::vector<int> get_parent_indices(node_descriptor node) const {
            std::vector<int> parent_indices;
            auto it_parents = g.get_parent_edges(node);

            for (auto it = it_parents.first; it != it_parents.second; ++it) {
                parent_indices.push_back(g.index(g.source(*it)));
            }

            return parent_indices;
        }

        std::vector<int> get_parent_indices(int node_index) const {
            return get_parent_indices(g.node(node_index));
        }

        std::vector<int> get_parent_indices(const std::string& node) const {
            return get_parent_indices(m_indices.at(node));
        }

        bool has_edge(node_descriptor source, node_descriptor dest) const {
            return g.has_edge(source, dest);
        }

        bool has_edge(int source, int dest) const {
            return has_edge(g.node(source), g.node(dest));
        }

        bool has_edge(const std::string& source, const std::string& dest) const {
            return has_edge(m_indices.at(source), m_indices.at(dest));
        }

        bool has_path(node_descriptor source, node_descriptor dest) const {
            return g.has_path(source, dest);
        }
        
        bool has_path(int source_index, int dest_index) const {
            return has_path(g.node(source_index), g.node(dest_index));
        }
        
        bool has_path(const std::string& source, const std::string& dest) const {
            return has_path(m_indices.at(source), m_indices.at(dest));
        }

        void add_edge(node_descriptor source, node_descriptor dest) {
            g.add_edge(source, dest);
        }

        void add_edge(int source, int dest) {
            add_edge(g.node(source), g.node(dest), g);
        }

        void add_edge(const std::string& source, const std::string& dest) const {
            add_edge(m_indices.at(source), m_indices.at(dest));
        }

        bool can_add_edge(node_descriptor source, node_descriptor dest) const {
            if (num_parents(source) == 0 || num_children(dest) == 0 || !has_path(dest, source)) {
                return true;
            }

            return false;
        }

        bool can_add_edge(int source_index, int dest_index) const {
            return can_add_edge(node(source_index), node(dest_index));
        }

        bool can_add_edge(const std::string& source, const std::string& dest) const {
            return can_add_edge(m_indices.at(source), m_indices.at(dest));
        }

        bool can_flip_edge(node_descriptor source, node_descriptor dest) {
            if (num_parents(dest) == 0 || num_children(source) == 0) {
                return true;
            } else {
                remove_edge(source, dest);
                bool thereis_path = has_path(source, dest);
                add_edge(source, dest);
                if (thereis_path) {
                    return false;
                } else {
                    return true;
                }
            }
        }

        bool can_flip_edge(int source_index, int dest_index) {
            return can_flip_edge(node(source_index), node(dest_index));
        }

        bool can_flip_edge(const std::string& source, const std::string& dest) {
            return can_flip_edge(m_indices.at(source), m_indices.at(dest));
        }

        void remove_edge(node_descriptor source, node_descriptor dest) {
            g.remove_edge(source, dest);
        }

        void remove_edge(int source, int dest) {
            remove_edge(g.node(source), g.node(dest), g);
        }

        void remove_edge(const std::string& source, const std::string& dest) const {
            remove_edge(m_indices.at(source), m_indices.at(dest));
        }

        void print() const {
            std::cout << "Bayesian network: " << std::endl; 
            for(auto [eit, eend] = g.edges(); eit != eend; ++eit)
                std::cout << name(g.source(*eit)) << " -> " << name(g.target(*eit)) << std::endl;
        }

    private:
        DagType g;
        std::vector<std::string> m_nodes;
        // Change to FNV hash function?
        std::unordered_map<std::string, int> m_indices;
    };

    template<BayesianNetworkType T, typename DagType>
    BayesianNetwork<T, DagType>::BayesianNetwork(const std::vector<std::string>& nodes) : g(nodes.size()), m_nodes(nodes), m_indices(nodes.size()) {
        int i = 0;
        for (const std::string& str : nodes) {
            m_indices.insert(std::make_pair(str, i));
            ++i;
        }
    };

    template<BayesianNetworkType T, typename DagType>
    BayesianNetwork<T, DagType>::BayesianNetwork(const std::vector<std::string>& nodes, 
                                                 const arc_vector& edges) 
                                                 : g(nodes.size()), m_nodes(nodes), m_indices(nodes.size())
    {
        int i = 0;
        for (const std::string& str : nodes) {
            m_indices.insert(std::make_pair(str, i));
            ++i;
        }

        for(auto edge : edges) {
            g.add_edge(node(edge.first), node(edge.second));
        }

    };

    using GaussianNetwork = BayesianNetwork<BayesianNetworkType::GAUSSIAN_NETWORK>;
    using GaussianNetworkList = BayesianNetwork<BayesianNetworkType::GAUSSIAN_NETWORK, AdjListDag>;

}




#endif //PGM_DATASET_BAYESIANNETWORK_HPP