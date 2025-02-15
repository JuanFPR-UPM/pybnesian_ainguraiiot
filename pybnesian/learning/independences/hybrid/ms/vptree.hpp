#ifndef PYBNESIAN_LEARNING_INDEPENDENCES_HYBRID_MS_VPTREE_HPP
#define PYBNESIAN_LEARNING_INDEPENDENCES_HYBRID_MS_VPTREE_HPP

#include <dataset/dataset.hpp>
#include <queue>
#include <random>
#include <algorithm>

using dataset::DataFrame;
using Eigen::Matrix, Eigen::Dynamic, Eigen::VectorXd, Eigen::VectorXi;

template <typename ArrowType>
using DowncastArray_ptr = std::shared_ptr<typename arrow::TypeTraits<ArrowType>::ArrayType>;

template <typename ArrowType>
using DowncastArray_vector = std::vector<DowncastArray_ptr<ArrowType>>;

namespace vptree {

template <typename ArrowType>
class HybridChebyshevDistance {
public:
    using CType = typename ArrowType::c_type;
    using ArrayType = typename arrow::TypeTraits<ArrowType>::ArrayType;
    using OperationFunc = std::function<CType(size_t, size_t)>;

    HybridChebyshevDistance(const std::vector<std::shared_ptr<ArrayType>>& data,
                            const std::vector<bool>& is_discrete_column)
        : m_data(data) {
        m_operations_coords.reserve(m_data.size());
        for (size_t i = 0; i < m_data.size(); ++i) {
            if (is_discrete_column[i]) {
                // For discrete columns, Hamming distance
                m_operations_coords.push_back([this, i](size_t p1_index, size_t p2_index) -> CType {
                    return (m_data[i]->Value(p1_index) != m_data[i]->Value(p2_index));
                });
            } else {
                // For continuous columns, Manhattan distance
                m_operations_coords.push_back([this, i](size_t p1_index, size_t p2_index) -> CType {
                    return std::abs(m_data[i]->Value(p1_index) - m_data[i]->Value(p2_index));
                });
            }
        }
    }

    inline CType distance(size_t p1_index, size_t p2_index) const {
        CType d = 0;
        for (auto it_operation = m_operations_coords.begin(), it_end = m_operations_coords.end();
             it_operation != it_end;
             ++it_operation) {
            d = std::max(d, (*it_operation)(p1_index, p2_index));
        }

        return d;
    }

    inline CType distance_coords(size_t p1_index, size_t p2_index, std::vector<int>& coords) const {
        CType d = 0;
        for (auto it_col_idx = coords.begin(); it_col_idx != coords.end(); it_col_idx++) {
            d = std::max(d, m_operations_coords[*it_col_idx](p1_index, p2_index));
        }

        return d;
    }

    inline CType distance_p(CType difference) const { return std::abs(difference); }

    inline CType normalize(CType nonnormalized) const { return nonnormalized; }

    inline CType update_component_distance(CType distance, CType, CType new_component) const {
        return std::max(distance, new_component);
    }

private:
    const std::vector<std::shared_ptr<ArrayType>>& m_data;
    std::vector<OperationFunc> m_operations_coords;
};

struct VPTreeNode {
    size_t index;
    double threshold;
    std::unique_ptr<VPTreeNode> left;
    std::unique_ptr<VPTreeNode> right;
    std::vector<size_t> leaf_indices;
    bool is_leaf;
};

class VPTree {
public:
    VPTree(DataFrame& df,
           std::shared_ptr<arrow::DataType> datatype,
           std::vector<bool>& is_discrete_column,
           int leafsize = 16,
           unsigned int seed = std::random_device{}())
        : m_df(df),
          m_datatype(datatype),
          m_is_discrete_column(is_discrete_column),
          m_column_names(df.column_names()),
          m_root(),
          m_leafsize(leafsize),
          m_seed(seed) {
    
        m_root = build_vptree(m_df, m_datatype, m_is_discrete_column, m_leafsize, m_seed);
    }

    std::vector<std::pair<VectorXd, VectorXi>> query(const DataFrame& test_df, int k) const;

    template <typename ArrowType>
    std::pair<VectorXd, VectorXi> query_instance(size_t i,
                                                 int k,
                                                 const HybridChebyshevDistance<ArrowType>& distance) const;

    std::tuple<VectorXi, VectorXi, VectorXi> count_ball_subspaces(const DataFrame& test_df,
                                                                  const VectorXd& eps,
                                                                  std::vector<bool>& is_discrete_column) const;

    template <typename ArrowType>
    std::tuple<int, int, int> count_ball_subspaces_instance(size_t i,
                                                            const typename ArrowType::c_type eps_value,
                                                            const HybridChebyshevDistance<ArrowType>& distance) const;

    VectorXi count_ball_unconditional(const DataFrame& test_df,
                                      const VectorXd& eps,
                                      std::vector<bool>& is_discrete_column) const;

    template <typename ArrowType>
    int count_ball_unconditional_instance(size_t i,
                                          const typename ArrowType::c_type eps_value,
                                          const HybridChebyshevDistance<ArrowType>& distance) const;

    const DataFrame& scaled_data() const { return m_df; }

private:
    std::unique_ptr<VPTreeNode> build_vptree(const DataFrame& df,
                                             const std::shared_ptr<arrow::DataType> datatype,
                                             const std::vector<bool>& is_discrete_column,
                                             int leafsize,
                                             unsigned int seed);

    DataFrame& m_df;
    std::shared_ptr<arrow::DataType> m_datatype;
    std::vector<bool>& m_is_discrete_column;
    std::vector<std::string> m_column_names;
    std::unique_ptr<VPTreeNode> m_root;
    int m_leafsize;
    unsigned int m_seed;
};

}  // namespace vptree

#endif  // PYBNESIAN_LEARNING_INDEPENDENCES_HYBRID_MS_VPTREE_HPP