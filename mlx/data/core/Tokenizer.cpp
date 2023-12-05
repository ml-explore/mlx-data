// Copyright © 2023 Apple Inc.

#include <deque>
#include <stdexcept>
#include <string>
#include <tuple>
#include <vector>

#include "mlx/data/core/Graph.h"
#include "mlx/data/core/State.h"
#include "mlx/data/core/Tokenizer.h"
#include "mlx/data/core/Trie.h"

namespace mlx {
namespace data {
namespace core {

std::shared_ptr<Graph<int64_t>> tokenize(
    std::shared_ptr<const Trie<char>> trie,
    const std::string& input,
    bool ignore_unk) {
  Graph<int64_t> tokens;

  // (trie, node id in tokens, word label)
  std::deque<std::tuple<const TrieNode<char>*, int64_t, int64_t>> hyps;
  std::tuple<const TrieNode<char>*, int64_t, int64_t> last_word_hyp = {
      trie->root(), tokens.add_node(), -1};

  hyps.push_back(last_word_hyp);
  tokens.start_node(std::get<1>(hyps.back()));
  for (int t = 0; t < input.size(); t++) {
    std::deque<std::tuple<const TrieNode<char>*, int64_t, int64_t>> new_hyps;
    for (auto& hyp : hyps) {
      auto trie_node = std::get<0>(hyp);
      auto word_node_id = std::get<1>(hyp);
      auto kv = trie_node->children.find(input[t]);
      if (kv != trie_node->children.end()) {
        const TrieNode<char>* new_trie_node = kv->second;
        if (new_trie_node->accepts()) { // could be leaf or not
          // word hyps in front
          new_hyps.push_front({trie->root(), word_node_id, new_trie_node->id});
        }
        if (!new_trie_node->children.empty()) { // not a leaf
          // partial word hyps in back
          new_hyps.push_back({new_trie_node, word_node_id, -1});
        }
      }
    }
    if (new_hyps.empty()) {
      if (ignore_unk) {
        // abandon all partial hyps
        // so we have no choice but to go back to last generated word
        hyps = {last_word_hyp};
        continue;
      } else {
        throw std::runtime_error(
            "could not tokenize: <" + input + "> at position " +
            std::to_string(t));
      }
    }

    // merge identical nodes (trie->root() ones)
    // and update word graph accordingly
    if (std::get<0>(new_hyps.front()) == trie->root()) {
      auto new_word_node_id = tokens.add_node();
      while (!new_hyps.empty()) {
        auto& hyp = new_hyps.front();
        if (std::get<0>(hyp) != trie->root()) {
          break;
        }
        tokens.add_edge(std::get<1>(hyp), new_word_node_id, std::get<2>(hyp));
        new_hyps.pop_front();
      }
      last_word_hyp = {trie->root(), new_word_node_id, -1};
      new_hyps.push_front(last_word_hyp);
    }

    hyps = std::move(new_hyps);
  }

  // note: only one hyp should have trie->root()
  if (std::get<0>(hyps.front()) != trie->root()) {
    throw std::runtime_error("could not tokenize: <" + input + ">");
  }
  tokens.final_node(std::get<1>(hyps.front()));

  // Keep only reachable tokens
  auto valid_tokens = std::make_shared<Graph<int64_t>>();
  std::vector<int64_t> newid(tokens.num_nodes(), -1);

  auto visitor = [&tokens, valid_tokens, &newid](int64_t id) {
    if (newid[id] < 0) {
      newid[id] = valid_tokens->add_node();
    }
    for (auto edgeid : tokens.iedges(id)) {
      auto fromid = tokens.inode(edgeid);
      if (newid[fromid] < 0) {
        newid[fromid] = valid_tokens->add_node();
      }
      valid_tokens->add_edge(newid[fromid], newid[id], tokens.edge(edgeid));
    }
  };
  tokens.visit_nodes(
      std::vector{std::get<1>(hyps.front())},
      visitor,
      [](int64_t) { return true; },
      true);

  for (auto id : tokens.start_nodes()) {
    valid_tokens->start_node(newid[id]);
  }
  for (auto id : tokens.final_nodes()) {
    valid_tokens->final_node(newid[id]);
  }
  return valid_tokens;
}

Tokenizer::Tokenizer(
    std::shared_ptr<const Trie<char>> trie,
    bool ignore_unk,
    const std::vector<double>& trie_key_scores)
    : trie_(trie), ignoreUnk_(ignore_unk), trieKeyScores_(trie_key_scores) {
  if (!trie_key_scores.empty() &&
      (trie_key_scores.size() != trie->num_keys())) {
    throw std::runtime_error(
        "Tokenizer: trie keys and trie scores do not match");
  }
  trieKeyScoresPositive_ = true;
  for (auto score : trie_key_scores) {
    if (score < 0) {
      trieKeyScoresPositive_ = false;
      break;
    }
  }
}

std::shared_ptr<Graph<int64_t>> Tokenizer::tokenize(
    const std::string& input) const {
  return ::mlx::data::core::tokenize(trie_, input, ignoreUnk_);
}

std::vector<int64_t> Tokenizer::tokenize_shortest(
    const std::string& input) const {
  auto tok_g = tokenize(input);
  std::vector<double> edge_scores(tok_g->num_edges(), 1.0);
  if (!trieKeyScores_.empty()) {
    for (int64_t i = 0; i < edge_scores.size(); i++) {
      edge_scores[i] = trieKeyScores_.at(tok_g->edge(i));
    }
  }
  auto tokens = std::get<0>(tok_g->shortest_path(
      edge_scores,
      std::vector<double>(),
      false,
      nullptr,
      trieKeyScoresPositive_));
  for (int64_t i = 0; i < tokens.size(); i++) {
    tokens[i] = tok_g->edge(tokens[i]);
  }
  return tokens;
}

std::vector<int64_t> Tokenizer::tokenize_rand(const std::string& input) const {
  auto tok_g = tokenize(input);
  std::vector<int64_t> tokens;
  std::vector<bool> selected_edges(tok_g->num_edges(), false);
  auto select_edge = [&selected_edges, &tok_g](int64_t node_id) {
    auto& edges = tok_g->oedges(node_id);
    if (edges.size() > 0) {
      std::uniform_int_distribution<int64_t> uniform{
          0, static_cast<int64_t>(edges.size()) - 1};
      selected_edges[edges[uniform(get_state()->randomGenerator)]] = true;
    }
  };
  auto is_selected_edge = [&selected_edges, &tok_g, &tokens](int64_t edge_id) {
    if (selected_edges[edge_id]) {
      tokens.push_back(tok_g->edge(edge_id));
      return true;
    } else {
      return false;
    }
  };
  tok_g->visit_nodes(tok_g->start_nodes(), select_edge, is_selected_edge);
  return tokens;
}

TokenizerIterator::TokenizerIterator(std::shared_ptr<Graph<int64_t>> graph)
    : g_(graph) {
  edgeIndices_.push_back(0);
  startNodeIterator_ = g_->start_nodes().begin();
  new_start_();
}

bool TokenizerIterator::new_start_() {
  if (startNodeIterator_ != g_->start_nodes().end()) {
    currentNodeId_ = *startNodeIterator_++;
    forward_();
    return true;
  }
  return false;
}
void TokenizerIterator::forward_() {
  // go to end
  while (edgeIndices_.back() < g_->oedges(currentNodeId_).size()) {
    auto edgeid = g_->oedges(currentNodeId_)[edgeIndices_.back()];
    currentTokens_.push_back(g_->edge(edgeid));
    edgeIndices_.push_back(0);
    currentNodeId_ = g_->onode(edgeid);
    backEdgeIds_.push_back(edgeid);
  }
}

std::vector<int64_t> TokenizerIterator::next() {
  while (currentTokens_.empty() && new_start_())
    ;
  std::vector<int64_t> result = currentTokens_;
  if (!result.empty()) {
    while (edgeIndices_.back() == g_->oedges(currentNodeId_).size() &&
           (backEdgeIds_.size() > 0)) {
      currentNodeId_ = g_->inode(backEdgeIds_.back());
      backEdgeIds_.pop_back();
      edgeIndices_.pop_back();
      currentTokens_.pop_back();
      edgeIndices_.back()++;
    }
    forward_();
  }
  return result;
}

} // namespace core
} // namespace data
} // namespace mlx
