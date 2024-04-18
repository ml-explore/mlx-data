// Copyright © 2023 Apple Inc.

#pragma once

#include <cstdint>
#include <deque>
#include <iterator>
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

  template <typename iterator_type>
  std::tuple<const TrieNode<T>*, int64_t> search_longest_prefix(
      iterator_type it,
      iterator_type end) const {
    auto node = root_();
    int64_t i = 0;
    auto valid_node = node;
    int64_t valid_i = i;
    while (it != end) {
      auto kv = node->children.find(*it);
      if (kv == node->children.end()) {
        break;
      } else {
        node = kv->second;
        i++;
        it++;
        if (node->accepts()) {
          valid_node = node;
          valid_i = i;
        }
      }
    }
    return std::make_tuple(valid_node, valid_i);
  }

  template <typename iterator_type>
  const TrieNode<T>*
  insert(iterator_type begin, iterator_type end, int64_t id = -1) {
    id = (id < 0) ? keys_.size() : id;
    auto it = begin;
    auto [node, i] = partial_search(it, end);
    std::advance(it, i); // it += i but also supports sequential iterators
    while (it != end) {
      nodes_.resize(nodes_.size() + 1);
      TrieNode<T>* new_node = &nodes_.back();
      new_node->uid = nodes_.size() - 1;
      new_node->id = -1;
      node->children[*it] = new_node;
      node = new_node;
      it++;
    }
    if (!node->accepts()) {
      node->id = id;
      keys_.emplace(id, std::vector<T>(begin, end));
    }
    return node;
  }

  template <typename iterator_type>
  const TrieNode<T>* search(iterator_type it, iterator_type end) {
    auto [node, i] = partial_search(it, end);
    if (i != std::distance(it, end) || !node->accepts()) {
      return nullptr;
    }
    return node;
  }

  const TrieNode<T>* insert(const std::vector<T>& key, int64_t id = -1) {
    return insert(key.begin(), key.end(), id);
  }

  const TrieNode<T>* search(const std::vector<T>& key) {
    return search(key.begin(), key.end());
  }

  const TrieNode<T>* root() const {
    return &nodes_.front();
  }

  int64_t num_keys() const {
    return keys_.size();
  }

  const std::vector<T>& key(int64_t id) const {
    return keys_.at(id);
  }

  // helpers for strings
  template <
      typename U = T,
      std::enable_if_t<std::is_same<char, U>::value, char> = false>
  const TrieNode<T>* insert(const std::string& key, int64_t id = -1) {
    return insert(key.begin(), key.end(), id);
  };
  template <
      typename U = T,
      std::enable_if_t<std::is_same<char, U>::value, char> = false>
  const TrieNode<T>* search(const std::string& key) {
    return search(key.begin(), key.end());
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

  const TrieNode<T>* root_() const {
    return &nodes_.front();
  }

  template <typename iterator_type>
  std::tuple<TrieNode<T>*, int64_t> partial_search(
      iterator_type it,
      iterator_type end) {
    auto node = root_();
    int64_t i = 0;
    while (it != end) {
      auto kv = node->children.find(*it);
      if (kv == node->children.end()) {
        break;
      } else {
        node = kv->second;
        i++;
        it++;
      }
    }
    return std::make_tuple(node, i);
  }

  std::deque<TrieNode<T>> nodes_;
  std::unordered_map<int64_t, std::vector<T>> keys_;
};

} // namespace core
} // namespace data
} // namespace mlx
