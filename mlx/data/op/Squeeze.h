// Copyright © 2023 Apple Inc.

#pragma once

#include "mlx/data/op/KeyTransform.h"

namespace mlx {
namespace data {
namespace op {

class Squeeze : public KeyTransformOp {
 public:
  Squeeze(const std::string& ikey, const std::string& okey = "");
  Squeeze(const std::string& ikey, int dim, const std::string& okey = "");
  Squeeze(
      const std::string& ikey,
      const std::vector<int>& dims,
      const std::string& okey = "");

  virtual std::shared_ptr<Array> apply_key(
      const std::shared_ptr<const Array>& src) const override;

 private:
  std::vector<int> dims_;
};

} // namespace op
} // namespace data
} // namespace mlx
