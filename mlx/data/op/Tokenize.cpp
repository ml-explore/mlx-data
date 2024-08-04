// Copyright © 2023 Apple Inc.

#include "mlx/data/op/Tokenize.h"

namespace mlx {
namespace data {
namespace op {

Tokenize::Tokenize(
    const std::string& ikey,
    std::shared_ptr<core::Trie<char>> trie,
    TokenizeMode mode,
    bool ignore_unk,
    const std::vector<double>& trie_key_scores,
    const std::string& okey)
    : KeyTransformOp(ikey, okey),
      tokenizer_(trie, ignore_unk, trie_key_scores),
      mode_(mode) {}

std::shared_ptr<Array> Tokenize::apply_key(
    const std::shared_ptr<const Array>& src) const {
  std::string str(
      reinterpret_cast<char*>(src->data()), src->size() * src->itemsize());

  std::vector<int64_t> tokens;
  switch (mode_) {
    case TokenizeMode::shortest:
      tokens = tokenizer_.tokenize_shortest(str);
      break;
    case TokenizeMode::rand:
      tokens = tokenizer_.tokenize_rand(str);
      break;
    default:
      throw std::runtime_error("Tokenize: unsupported tokenize mode");
  }

  return std::make_shared<Array>(tokens);
}

BPETokenize::BPETokenize(
    const std::string& ikey,
    std::shared_ptr<const core::Trie<char>> symbols,
    std::shared_ptr<const core::BPEMerges> merges,
    const std::string& okey)
    : KeyTransformOp(ikey, okey), tokenizer_(symbols, merges) {}

std::shared_ptr<Array> BPETokenize::apply_key(
    const std::shared_ptr<const Array>& src) const {
  auto tokens = tokenizer_.tokenize(std::string_view(
      reinterpret_cast<char*>(src->data()), src->size() * src->itemsize()));
  return std::make_shared<Array>(tokens);
}

} // namespace op
} // namespace data
} // namespace mlx
