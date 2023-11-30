// Copyright © 2023 Apple Inc.

#pragma once

#include <memory>
#include <string>

#include "mlx/data/core/Graph.h"
#include "mlx/data/core/Trie.h"

namespace mlx {
namespace data {
namespace core {

std::shared_ptr<Graph<int64_t>> tokenize(
    std::shared_ptr<const Trie<char>> trie,
    const std::string& input,
    bool ignore_unk = false);

class Tokenizer {
 public:
  Tokenizer(
      std::shared_ptr<const Trie<char>> trie,
      bool ignore_unk = false,
      const std::vector<double>& trie_key_scores = {});
  std::shared_ptr<Graph<int64_t>> tokenize(const std::string& input) const;
  std::vector<int64_t> tokenize_shortest(const std::string& input) const;
  std::vector<int64_t> tokenize_rand(const std::string& input) const;

 private:
  std::shared_ptr<const Trie<char>> trie_;
  bool ignoreUnk_;
  std::vector<double> trieKeyScores_;
  bool trieKeyScoresPositive_;
};

class TokenizerIterator {
 public:
  TokenizerIterator(std::shared_ptr<Graph<int64_t>> graph);
  std::vector<int64_t> next();

 private:
  std::shared_ptr<Graph<int64_t>> g_;
  std::vector<int64_t> edgeIndices_; // edge indices for path so far
  std::vector<int64_t> backEdgeIds_; // back edge ids for path so far
  int64_t currentNodeId_; // current node
  std::vector<int64_t> currentTokens_; // current tokenization
  std::unordered_set<int64_t>::const_iterator startNodeIterator_;
  bool new_start_();
  void forward_();
};

} // namespace core
} // namespace data
} // namespace mlx
