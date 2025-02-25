// Copyright © 2024 Apple Inc.

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

std::vector<int64_t> BPETokenizer::tokenize(std::string_view input) const {
  struct Symbol {
    std::string_view value;
    int left;
    int right;
    int64_t token;
  };

  struct Pair {
    std::vector<Symbol>::iterator left;
    std::vector<Symbol>::iterator right;
    int64_t token;
    std::string_view value;

    Pair(
        std::vector<Symbol>::iterator left,
        std::vector<Symbol>::iterator right,
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
  std::vector<Symbol> symbols;
  symbols.reserve(input.size());
  for (auto it = input.begin(); it != input.end(); it++) {
    auto [node, length] = symbols_->search_longest_prefix(it, input.end());
    if (length == 0) {
      std::ostringstream msg;
      msg << "BPETokenizer: Unknown symbol '" << *it << "'";
      throw std::runtime_error(msg.str());
    }
    symbols.push_back(Symbol{
        std::string_view(&*it, length),
        static_cast<int>(symbols.size() - 1),
        static_cast<int>(symbols.size() + 1),
        node->id});
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
    Pair top = std::move(merge_queue.top());
    merge_queue.pop();

    // Skip invalidated pairs
    if (top.left->token < 0 || top.right->token < 0) {
      continue;
    }
    if (top.value.size() != top.left->value.size() + top.right->value.size()) {
      continue;
    }
    if (top.value.data() != top.left->value.data()) {
      continue;
    }

    // Yay! Valid pair, let's merge into the left one.
    top.left->token = top.token;
    top.left->value = top.value;

    // Invalidate our neighbor which we just merged into ourselves.
    top.right->token = -1;

    // Adjust the pointers to neighboring symbols
    top.left->right = top.right->right;
    if (top.right->right < symbols.size()) {
      symbols[top.right->right].left = top.right->left;
    }

    // Check for a possible merge to the left.
    if (top.left != symbols.begin()) {
      auto neighbor = symbols.begin() + top.left->left;
      auto [can_merge, token] =
          merges_->can_merge(neighbor->value, top.left->value);
      if (can_merge) {
        merge_queue.emplace(neighbor, top.left, token);
      }
    }

    // Do the same to our right.
    if (top.left->right < symbols.size()) {
      auto neighbor = symbols.begin() + top.left->right;
      auto [can_merge, token] =
          merges_->can_merge(top.left->value, neighbor->value);
      if (can_merge) {
        merge_queue.emplace(top.left, neighbor, token);
      }
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
