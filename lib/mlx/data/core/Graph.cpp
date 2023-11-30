// Copyright © 2023 Apple Inc.

#include <limits>
#include <queue>
#include <sstream>

#include "mlx/data/core/Graph.h"

namespace mlx {
namespace data {
namespace core {
GraphBase::GraphBase() {}
int64_t GraphBase::add_node_() {
  int64_t id = nodes_.size();
  nodes_.resize(id + 1);
  return id;
}
int64_t GraphBase::add_edge_(int64_t inode, int64_t onode) {
  if (inode < 0 || inode >= nodes_.size()) {
    throw std::runtime_error("Graph: invalid input node");
  }
  if (onode < 0 || onode >= nodes_.size()) {
    throw std::runtime_error("Graph: invalid output node");
  }
  int64_t id = edges_.size();
  nodes_[inode].oedges.push_back(id);
  nodes_[onode].iedges.push_back(id);
  edges_.resize(id + 1, {inode, onode});
  return id;
}
std::string GraphBase::dot_graph(
    const std::function<std::string(int64_t)>& nodelabel,
    const std::function<std::string(int64_t)>& edgelabel) const {
  std::ostringstream f;
  f << "digraph {" << std::endl;
  for (int64_t nodeid = 0; nodeid < nodes_.size(); nodeid++) {
    auto& node = nodes_[nodeid];
    std::string style = "";
    if (is_start_node(nodeid)) {
      style += " style=\"bold\"";
    }
    if (is_final_node(nodeid)) {
      style += " shape=\"doublecircle\"";
    }
    auto label = nodelabel(nodeid);
    if (!style.empty() || !label.empty()) {
      f << "S" << nodeid;
      f << " [";
      if (!label.empty()) {
        f << " label=\""
          << "S" << nodeid << " " << label << "\"";
      }
      if (!style.empty()) {
        f << style;
      }
      f << "];" << std::endl;
    }
    for (auto& edgeid : node.iedges) {
      f << "S" << edges_[edgeid].inode << " -> "
        << "S" << edges_[edgeid].onode;
      auto label = edgelabel(edgeid);
      if (!label.empty()) {
        f << " [label=\"" << label << "\"];" << std::endl;
      } else {
        f << ";" << std::endl;
      }
    }
  }
  f << "}";
  return f.str();
}
void GraphBase::merge(int64_t nodeid1, int64_t nodeid2) {
  auto& node1 = nodes_.at(nodeid1);
  auto& node2 = nodes_.at(nodeid2);
  for (auto& edgeid : node2.iedges) {
    edges_[edgeid].onode = nodeid1;
    // self-loop
    if (edges_[edgeid].inode == nodeid2) {
      edges_[edgeid].inode = nodeid1;
    }
  }
  for (auto& edgeid : node2.oedges) {
    edges_[edgeid].inode = nodeid1;
    // self-loop
    if (edges_[edgeid].onode == nodeid2) {
      edges_[edgeid].onode = nodeid1;
    }
  }
  node1.iedges.insert(
      node1.iedges.begin(), node2.iedges.begin(), node2.iedges.end());
  node1.oedges.insert(
      node1.oedges.begin(), node2.oedges.begin(), node2.oedges.end());
  node2.iedges.clear();
  node2.oedges.clear();
}
int64_t GraphBase::inode(int64_t edgeid) const {
  return edges_.at(edgeid).inode;
}
int64_t GraphBase::onode(int64_t edgeid) const {
  return edges_.at(edgeid).onode;
}
const std::vector<int64_t>& GraphBase::iedges(int64_t nodeid) const {
  return nodes_.at(nodeid).iedges;
}
const std::vector<int64_t>& GraphBase::oedges(int64_t nodeid) const {
  return nodes_.at(nodeid).oedges;
}
std::vector<bool> GraphBase::visit_nodes(
    std::vector<int64_t> nodes,
    const std::function<void(int64_t)>& node_visitor,
    const std::function<bool(int64_t)>& edge_visitor,
    bool reverse) const {
  std::vector<bool> visited(nodes_.size(), false);
  while (!nodes.empty()) {
    std::vector<int64_t> newnodes;
    for (auto node : nodes) {
      if (!visited.at(node)) {
        node_visitor(node);
        visited[node] = true;
        const std::vector<int64_t>& edges =
            (reverse ? iedges(node) : oedges(node));
        for (auto edge : edges) {
          if (edge_visitor(edge)) {
            newnodes.push_back((reverse ? inode(edge) : onode(edge)));
          }
        }
      }
    }
    nodes = std::move(newnodes);
  }
  return visited;
}
std::vector<bool> GraphBase::visit_nodes(
    std::unordered_set<int64_t> nodes,
    const std::function<void(int64_t)>& node_visitor,
    const std::function<bool(int64_t)>& edge_visitor,
    bool reverse) const {
  std::vector<int64_t> vnodes(nodes.begin(), nodes.end());
  return visit_nodes(vnodes, node_visitor, edge_visitor, reverse);
}
void GraphBase::start_node(int64_t id) {
  if (id < 0 || id >= nodes_.size()) {
    throw std::runtime_error("Graph: invalid node id");
  }
  startNodeIds_.insert(id);
}

void GraphBase::final_node(int64_t id) {
  if (id < 0 || id >= nodes_.size()) {
    throw std::runtime_error("Graph: invalid node id");
  }
  finalNodeIds_.insert(id);
}

bool GraphBase::is_start_node(int64_t id) const {
  return startNodeIds_.find(id) != startNodeIds_.end();
}
bool GraphBase::is_final_node(int64_t id) const {
  return finalNodeIds_.find(id) != finalNodeIds_.end();
}

const std::unordered_set<int64_t>& GraphBase::start_nodes() const {
  return startNodeIds_;
}
const std::unordered_set<int64_t>& GraphBase::final_nodes() const {
  return finalNodeIds_;
}

struct ShortestPathNode {
  int64_t nodeId;
  int64_t edgeId;
  double dist;
};
struct ShortestPath {
  ShortestPath(
      const GraphBase& g,
      const std::vector<double>& edge_weights,
      const std::vector<double>& node_weights,
      bool reverse)
      : g(g),
        edgeWeights(edge_weights),
        nodeWeights(node_weights),
        reverse(reverse){};

  virtual void find_from_start_node(
      int64_t start_node_id,
      std::vector<ShortestPathNode>& shortest) = 0;

  std::tuple<std::vector<int64_t>, std::vector<int64_t>> find(
      double* shortest_dist_ptr) {
    auto shortest_edge_path = std::vector<int64_t>();
    auto shortest_node_path = std::vector<int64_t>();
    auto shortest_dist = std::numeric_limits<double>::max();
    auto nodes = (reverse ? g.final_nodes() : g.start_nodes());
    for (auto start_node_id : nodes) {
      // best (source, dist) vector
      std::vector<ShortestPathNode> shortest(
          g.num_nodes(), {0, -1, std::numeric_limits<double>::max()});
      shortest.at(start_node_id) = {
          -1, -1, (nodeWeights.empty() ? 0.0 : nodeWeights.at(start_node_id))};

      find_from_start_node(start_node_id, shortest);

      // check all valid final nodes
      int64_t best_last_node = -1;
      double best_last_node_dist = std::numeric_limits<double>::max();
      auto& last_nodes = (reverse ? g.start_nodes() : g.final_nodes());
      for (auto id : last_nodes) {
        if (shortest[id].dist < best_last_node_dist) {
          best_last_node = id;
          best_last_node_dist = shortest[id].dist;
        }
      }

      // we found something better?
      if (best_last_node_dist < shortest_dist) {
        // backtrack the path
        std::vector<int64_t> reverse_edge_path;
        std::vector<int64_t> reverse_node_path;
        while (best_last_node >= 0) {
          reverse_node_path.push_back(best_last_node);
          if (shortest[best_last_node].edgeId >= 0) {
            reverse_edge_path.push_back(shortest[best_last_node].edgeId);
          }
          best_last_node = shortest[best_last_node].nodeId;
        }
        // record everything
        shortest_dist = best_last_node_dist;
        shortest_node_path.resize(reverse_node_path.size());
        std::reverse_copy(
            reverse_node_path.begin(),
            reverse_node_path.end(),
            shortest_node_path.begin());
        shortest_edge_path.resize(reverse_edge_path.size());
        std::reverse_copy(
            reverse_edge_path.begin(),
            reverse_edge_path.end(),
            shortest_edge_path.begin());
      }
    }

    if (shortest_dist_ptr) {
      *shortest_dist_ptr = shortest_dist;
    }
    return std::make_tuple(
        std::move(shortest_edge_path), std::move(shortest_node_path));
  }

  const GraphBase& g;
  const std::vector<double>& edgeWeights;
  const std::vector<double>& nodeWeights;
  bool reverse;
};

struct BellmanFordShortestPath : public ShortestPath {
  BellmanFordShortestPath(
      const GraphBase& g,
      const std::vector<double>& edge_weights,
      const std::vector<double>& node_weights,
      bool reverse)
      : ShortestPath(g, edge_weights, node_weights, reverse){};
  virtual void find_from_start_node(
      int64_t start_node_id,
      std::vector<ShortestPathNode>& shortest) override {
    bool has_changed = true;
    for (int64_t iter = 0; (iter < g.num_nodes()) && has_changed; iter++) {
      has_changed = bellman_ford_iter(shortest);
    }
    if (has_changed) {
      throw std::runtime_error(
          "BellmanFordShortestPath: detected negative-weight cycle in graph");
    }
  };
  bool bellman_ford_iter(std::vector<ShortestPathNode>& shortest) {
    auto has_changed = false;
    for (int64_t edge_id = 0; edge_id < g.num_edges(); edge_id++) {
      int64_t u, v;
      double weight = (edgeWeights.empty() ? 0.0 : edgeWeights.at(edge_id));
      if (reverse) {
        u = g.onode(edge_id);
        v = g.inode(edge_id);
      } else {
        u = g.inode(edge_id);
        v = g.onode(edge_id);
      }
      weight += (nodeWeights.empty() ? 0.0 : nodeWeights[v]);
      if (shortest.at(u).dist + weight < shortest.at(v).dist) {
        has_changed = true;
        shortest.at(v).dist = shortest.at(u).dist + weight;
        shortest.at(v).nodeId = u;
        shortest.at(v).edgeId = edge_id;
      }
    }
    return has_changed;
  }
};

struct DijsktraShortestPath : public ShortestPath {
  DijsktraShortestPath(
      const GraphBase& g,
      const std::vector<double>& edge_weights,
      const std::vector<double>& node_weights,
      bool reverse)
      : ShortestPath(g, edge_weights, node_weights, reverse){};
  virtual void find_from_start_node(
      int64_t start_node_id,
      std::vector<ShortestPathNode>& shortest) override {
    auto compare_shortest_path_node = [](const ShortestPathNode& a,
                                         const ShortestPathNode& b) {
      return a.dist > b.dist;
    };

    std::priority_queue<
        ShortestPathNode,
        std::vector<ShortestPathNode>,
        decltype(compare_shortest_path_node)>
        pq(compare_shortest_path_node);

    pq.push(ShortestPathNode(
        {start_node_id,
         -1,
         (nodeWeights.empty() ? 0.0 : nodeWeights[start_node_id])}));

    while (!pq.empty()) {
      // top() is the smallest distance in pq
      int u = pq.top().nodeId;
      pq.pop();

      // go through all adjacent nodes
      auto edges = (reverse ? g.iedges(u) : g.oedges(u));
      for (auto edge_id : edges) {
        int v = (reverse ? g.inode(edge_id) : g.onode(edge_id));
        double weight = (edgeWeights.empty() ? 0.0 : edgeWeights[edge_id]) +
            (nodeWeights.empty() ? 0.0 : nodeWeights[v]);

        // update shortest if there is shorter path to v from u
        if (shortest[v].dist > shortest[u].dist + weight) {
          shortest[v] = {u, edge_id, shortest[u].dist + weight};
          pq.push(ShortestPathNode({v, -1, shortest[v].dist}));
        }
      }
    }
  }
};

std::tuple<std::vector<int64_t>, std::vector<int64_t>> GraphBase::shortest_path(
    const std::vector<double>& edge_weights,
    const std::vector<double>& node_weights,
    bool reverse,
    double* shortest_distance,
    bool assume_positive_weights) {
  if (!edge_weights.empty() && edge_weights.size() != edges_.size()) {
    throw std::runtime_error("Graph: inconsistent edge weight size");
  }
  if (!node_weights.empty() && node_weights.size() != nodes_.size()) {
    throw std::runtime_error("Graph: inconsistent node weight size");
  }

  bool positive_weights = true;
  if (!assume_positive_weights) {
    for (auto weight : edge_weights) {
      if (weight < 0) {
        positive_weights = false;
        break;
      }
    }
    if (positive_weights) {
      for (auto weight : node_weights) {
        if (weight < 0) {
          positive_weights = false;
          break;
        }
      }
    }
  }
  if (positive_weights) {
    DijsktraShortestPath shortest_path(
        *this, edge_weights, node_weights, reverse);
    return shortest_path.find(shortest_distance);
  } else {
    BellmanFordShortestPath shortest_path(
        *this, edge_weights, node_weights, reverse);
    return shortest_path.find(shortest_distance);
  }
}

} // namespace core
} // namespace data
} // namespace mlx
