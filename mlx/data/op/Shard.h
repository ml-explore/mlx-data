// Copyright © 2023 Apple Inc.

#pragma once

#include "mlx/data/op/KeyTransform.h"

namespace mlx {
namespace data {
namespace op {

class Shard : public KeyTransformOp {
 public:
  Shard(
      const std::string& ikey,
      int64_t n_shards,
      const std::string& okey = "");

  virtual std::shared_ptr<Array> apply_key(
      const std::shared_ptr<const Array>& src) const override;

 private:
  int64_t nShards_;
};

} // namespace op
} // namespace data
} // namespace mlx
