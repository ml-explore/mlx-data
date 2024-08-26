// Copyright © 2023 Apple Inc.

#pragma once

#include <functional>
#include <string>
#include <unordered_set>
#include <vector>

namespace mlx {
namespace data {
namespace core {

struct EdgeBase {
  int64_t inode;
  int64_t onode;
};

struct NodeBase {
  std::vector<int64_t> iedges;
  std::vector<int64_t> oedges;
};

class GraphBase {
 public:
  GraphBase();

  std::string dot_graph(
      const std::function<std::string(int64_t)>& nodelabel =
          [](int64_t) { return std::string(); },
      const std::function<std::string(int64_t)>& edgelabel =
          [](int64_t) { return std::string(); }) const;

  void merge(int64_t node1, int64_t node2);
  int64_t inode(int64_t edgeid) const;
  int64_t onode(int64_t edgeid) const;
  const std::vector<int64_t>& iedges(int64_t nodeid) const;
  const std::vector<int64_t>& oedges(int64_t nodeid) const;
  std::vector<bool> visit_nodes(
      std::vector<int64_t> fromnodeids,
      const std::function<void(int64_t)>& node_visitor = [](int64_t) {},
      const std::function<bool(int64_t)>& edge_visitor =
          [](int64_t) { return true; },
      bool reverse = false) const;
  std::vector<bool> visit_nodes(
      std::unordered_set<int64_t> fromnodeids,
      const std::function<void(int64_t)>& node_visitor = [](int64_t) {},
      const std::function<bool(int64_t)>& edge_visitor =
          [](int64_t) { return true; },
      bool reverse = false) const;
  int64_t num_nodes() const {
    return nodes_.size();
  };
  int64_t num_edges() const {
    return edges_.size();
  };
  void start_node(int64_t id);
  void final_node(int64_t id);
  bool is_start_node(int64_t id) const;
  bool is_final_node(int64_t id) const;
  const std::unordered_set<int64_t>& start_nodes() const;
  const std::unordered_set<int64_t>& final_nodes() const;

  std::tuple<std::vector<int64_t>, std::vector<int64_t>> shortest_path(
      const std::vector<double>& edge_weights = std::vector<double>(),
      const std::vector<double>& node_weights = std::vector<double>(),
      bool reverse = false,
      double* shortest_distance = nullptr,
      bool assume_positive_weights = false);

 protected:
  int64_t add_node_();
  int64_t add_edge_(int64_t inode, int64_t onode);

 private:
  std::vector<NodeBase> nodes_;
  std::vector<EdgeBase> edges_;
  std::unordered_set<int64_t> startNodeIds_;
  std::unordered_set<int64_t> finalNodeIds_;
};

// Graph -- a graph with payloads on both edges and nodes
template <class A = void, class N = void>
class Graph : public GraphBase {
 public:
  N& node(int64_t id) {
    return uNodes_[id];
  };
  A& edge(int64_t id) {
    return uEdges_[id];
  };
  int64_t add_node(const N& node) {
    auto id = add_node_();
    if (uNodes_.size() <= id) {
      uNodes_.resize(id + 1);
    }
    uNodes_[id] = node;
    return id;
  };
  int64_t add_edge(int64_t inode, int64_t onode, const A& edge) {
    auto id = add_edge_(inode, onode);
    if (uEdges_.size() <= id) {
      uEdges_.resize(id + 1);
    }
    uEdges_[id] = edge;
    return id;
  };

 private:
  std::vector<N> uNodes_;
  std::vector<A> uEdges_;
};

// Graph<void, void> -- a graph with no payload
template <>
class Graph<void, void> : public GraphBase {
 public:
  int64_t add_node() {
    return add_node_();
  };
  int64_t add_edge(int64_t inode, int64_t onode) {
    return add_edge_(inode, onode);
  };
};

// Graph<A, void> -- a graph with payload on edges
template <class A>
class Graph<A, void> : public GraphBase {
 public:
  Graph() : GraphBase(){};
  A& edge(int64_t id) {
    return uEdges_[id];
  };
  int64_t add_node() {
    return add_node_();
  };
  int64_t add_edge(int64_t inode, int64_t onode, const A& edge) {
    auto id = add_edge_(inode, onode);
    if (uEdges_.size() <= id) {
      uEdges_.resize(id + 1);
    }
    uEdges_[id] = edge;
    return id;
  };

 private:
  std::vector<A> uEdges_;
};

// Graph<void, N> -- a graph with payloads on nodes
template <class N>
class Graph<void, N> : public GraphBase {
 public:
  N& node(int64_t id) {
    return uNodes_[id];
  };
  int64_t add_node(const N& node) {
    auto id = add_node_();
    if (uNodes_.size() <= id) {
      uNodes_.resize(id + 1);
    }
    uNodes_[id] = node;
    return id;
  };
  int64_t add_edge(int64_t inode, int64_t onode) {
    return add_edge_(inode, onode);
  };

 private:
  std::vector<N> uNodes_;
};

} // namespace core
} // namespace data
} // namespace mlx
