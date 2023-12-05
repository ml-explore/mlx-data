// Copyright © 2023 Apple Inc.

#pragma once

#include <cstdint>
#include <deque>
#include <string>
#include <tuple>
#include <unordered_map>
#include <vector>

namespace mlx {
namespace data {
namespace core {

// could have labels (transducer...) or we could have a map from TrieNode
// to whatever
template <class T>
struct TrieNode {
  std::unordered_map<T, TrieNode*> children;
  int64_t uid;
  int64_t id;
  bool accepts() const {
    return id >= 0;
  }
};

template <class T>
class Trie {
 public:
  Trie() {
    nodes_.resize(1);
    nodes_.back().id = -1; // uid is 0
  };
  const TrieNode<T>* insert(const std::vector<T>& key) {
    TrieNode<T>* node;
    int64_t i;
    std::tie(node, i) = partial_search_(key);
    for (; i < key.size(); i++) {
      nodes_.resize(nodes_.size() + 1);
      TrieNode<T>* new_node = &nodes_.back();
      new_node->uid = nodes_.size() - 1;
      new_node->id = -1;
      node->children[key[i]] = new_node;
      node = new_node;
    }
    if (!node->accepts()) {
      node->id = keys_.size();
      keys_.push_back(key);
    }
    return node;
  };
  const TrieNode<T>* search(const std::vector<T>& key) {
    auto res = partial_search_(key);
    if (std::get<1>(res) != key.size()) {
      return nullptr;
    } else {
      auto node = std::get<0>(res);
      return (node->accepts() ? node : nullptr);
    }
  };
  const TrieNode<T>* root() const {
    return &nodes_.front();
  }
  int64_t num_keys() const {
    return keys_.size();
  }
  const std::vector<T>& key(int64_t id) const {
    return keys_.at(id);
  }

  // helper for strings
  template <
      typename U = T,
      std::enable_if_t<std::is_same<char, U>::value, char> = false>
  const TrieNode<T>* insert(const std::string& key) {
    return insert(std::vector<T>(key.begin(), key.end()));
  };
  template <
      typename U = T,
      std::enable_if_t<std::is_same<char, U>::value, char> = false>
  const TrieNode<T>* search(const std::string& key) {
    return search(std::vector<T>(key.begin(), key.end()));
  };
  template <
      typename U = T,
      std::enable_if_t<std::is_same<char, U>::value, char> = false>
  std::string key_string(int64_t id) const {
    auto k = key(id);
    return std::string(k.begin(), k.end());
  }

 private:
  TrieNode<T>* root_() {
    return &nodes_.front();
  }
  std::tuple<TrieNode<T>*, int64_t> partial_search_(const std::vector<T>& key) {
    auto node = root_();
    int64_t i = 0;
    for (; i < key.size(); i++) {
      auto kv = node->children.find(key[i]);
      if (kv == node->children.end()) {
        break;
      } else {
        node = kv->second;
      }
    }
    return std::make_tuple(node, i);
  }
  std::deque<TrieNode<T>> nodes_;
  std::vector<std::vector<T>> keys_;
};

} // namespace core
} // namespace data
} // namespace mlx
