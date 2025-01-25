// Copyright © 2024 Apple Inc.

#pragma once

#include <memory>
#include <unordered_map>
#include <unordered_set>

#include "mlx/data/core/Trie.h"

namespace mlx {
namespace data {
namespace core {

class BPEMerges {
 public:
  void add(const std::string& left, const std::string& right, int64_t token);
  std::pair<bool, int64_t> can_merge(
      std::string_view left,
      std::string_view right) const;

  template <typename iterator_type>
  std::pair<bool, int64_t>
  can_merge(iterator_type left, iterator_type middle, iterator_type end) const {
    // switch to std::string_view(left, middle) when in C++20
    return can_merge(
        std::string_view(&(*left), std::distance(left, middle)),
        std::string_view(&(*middle), std::distance(middle, end)));
  }

 private:
  std::unordered_set<std::string> strings_;
  std::unordered_map<
      std::string_view,
      std::unordered_map<std::string_view, int64_t>>
      merges_;
};

class BPETokenizer {
 public:
  BPETokenizer(
      std::shared_ptr<const Trie<char>> symbols,
      std::shared_ptr<const BPEMerges> merges);

  std::vector<int64_t> tokenize(std::string_view input) const;

 private:
  std::shared_ptr<const Trie<char>> symbols_;
  std::shared_ptr<const BPEMerges> merges_;
};

} // namespace core
} // namespace data
} // namespace mlx
