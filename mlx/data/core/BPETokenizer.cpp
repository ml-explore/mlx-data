// Copyright © 2024 Apple Inc.

#include <iostream>
#include <list>
#include <queue>
#include <sstream>

#include "mlx/data/core/BPETokenizer.h"
#include "mlx/data/core/Trie.h"

namespace mlx {
namespace data {
namespace core {

void BPEMerges::add(
    const std::string& left,
    const std::string& right,
    int64_t token) {
  auto [left_s, left_inserted] = strings_.insert(left);
  auto [right_s, right_inserted] = strings_.insert(right);

  std::string_view left_v(*left_s);
  std::string_view right_v(*right_s);

  auto left_it = merges_.find(left_v);
  if (left_it == merges_.end()) {
    merges_[left_v][right_v] = token;
  } else {
    auto right_it = left_it->second.find(right_v);
    if (right_it == left_it->second.end()) {
      left_it->second[right_v] = token;
    } else {
      right_it->second = std::min(token, right_it->second);
    }
  }
}

std::pair<bool, int64_t> BPEMerges::can_merge(
    std::string_view left,
    std::string_view right) const {
  auto left_it = merges_.find(left);
  if (left_it == merges_.end()) {
    return {false, 0};
  }
  auto right_it = left_it->second.find(right);
  if (right_it == left_it->second.end()) {
    return {false, 0};
  }
  return {true, right_it->second};
}

BPETokenizer::BPETokenizer(
    std::shared_ptr<const Trie<char>> symbols,
    std::shared_ptr<const BPEMerges> merges)
    : symbols_(symbols), merges_(merges) {}

std::vector<int64_t> BPETokenizer::tokenize(const std::string& input) const {
  struct Symbol {
    std::string_view value;
    int64_t token;
  };

  struct Pair {
    std::list<Symbol>::iterator left;
    std::list<Symbol>::iterator right;
    int64_t token;
    std::string_view value;

    Pair(
        std::list<Symbol>::iterator left,
        std::list<Symbol>::iterator right,
        int64_t token)
        : left(left),
          right(right),
          token(token),
          value(left->value.data(), left->value.size() + right->value.size()) {}

    bool operator<(const Pair& right) const {
      return token >= right.token;
    };
  };

  // Transform the input to a sequence of basic symbols that will subsequently
  // be merged.
  std::list<Symbol> symbols;
  for (auto it = input.begin(); it != input.end(); it++) {
    auto [node, length] = symbols_->search_longest_prefix(it, input.end());
    if (length == 0) {
      std::ostringstream msg;
      msg << "BPETokenizer: Unknown symbol '" << *it << "'";
      throw std::runtime_error(msg.str());
    }
    symbols.push_back(Symbol{std::string_view(&*it, length), node->id});
    it += length - 1;
  }

  std::priority_queue<Pair> merge_queue;

  // Initialize the merge queue
  auto left = symbols.begin();
  auto right = std::next(left);
  while (right != symbols.end()) {
    auto [can_merge, token] = merges_->can_merge(left->value, right->value);
    if (can_merge) {
      merge_queue.emplace(left, right, token);
    }
    left++;
    right++;
  }

  while (!merge_queue.empty()) {
    Pair pair = std::move(merge_queue.top());
    merge_queue.pop();

    // If both left and right are valid and the value matches the pair value it
    // means we can merge freely.
    if (pair.left->token >= 0 && pair.right->token >= 0 &&
        pair.value.size() ==
            pair.left->value.size() + pair.right->value.size() &&
        pair.value.data() == pair.left->value.data()) {
      pair.left->token = pair.token;
      pair.left->value = pair.value;
      pair.right->token = -1;
      continue;
    }

    // This means that the pair is invalid for some reason, so we "eat" left
    // and right until they both have valid symbols. Subsequently we push the
    // new pair in the merge queue.
    while (pair.left->token == -1) {
      pair.left = symbols.erase(pair.left);
      pair.left--;
    }
    while (pair.right != symbols.end() && pair.right->token == -1) {
      pair.right = symbols.erase(pair.right);
    }

    auto [can_merge, token] =
        merges_->can_merge(pair.left->value, pair.right->value);
    if (can_merge) {
      merge_queue.emplace(pair.left, pair.right, token);
    }
  }

  // Gather the final result in a vector
  std::vector<int64_t> tokens;
  for (auto& symbol : symbols) {
    if (symbol.token >= 0) {
      tokens.push_back(symbol.token);
    }
  }

  return tokens;
}

} // namespace core
} // namespace data
} // namespace mlx
