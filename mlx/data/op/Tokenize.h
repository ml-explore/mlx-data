// Copyright © 2023 Apple Inc.

#pragma once

#include "mlx/data/core/BPETokenizer.h"
#include "mlx/data/core/Tokenizer.h"
#include "mlx/data/core/Trie.h"
#include "mlx/data/op/KeyTransform.h"

namespace mlx {
namespace data {
namespace op {

enum class TokenizeMode { shortest, rand };

class Tokenize : public KeyTransformOp {
 public:
  Tokenize(
      const std::string& ikey,
      std::shared_ptr<core::Trie<char>> trie,
      TokenizeMode mode,
      bool ignore_unk = false,
      const std::vector<double>& trie_key_scores = {},
      const std::string& okey = "");

  virtual std::shared_ptr<Array> apply_key(
      const std::shared_ptr<const Array>& src) const override;

 private:
  core::Tokenizer tokenizer_;
  TokenizeMode mode_;
};

class BPETokenize : public KeyTransformOp {
 public:
  BPETokenize(
      const std::string& ikey,
      std::shared_ptr<const core::Trie<char>> symbols,
      std::shared_ptr<const core::BPEMerges> merges,
      const std::string& okey = "");

  virtual std::shared_ptr<Array> apply_key(
      const std::shared_ptr<const Array>& src) const override;

 private:
  core::BPETokenizer tokenizer_;
};

} // namespace op
} // namespace data
} // namespace mlx
