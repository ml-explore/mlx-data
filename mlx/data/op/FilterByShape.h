// Copyright © 2023 Apple Inc.

#pragma once

#include "mlx/data/op/Op.h"

namespace mlx {
namespace data {
namespace op {

class FilterByShape : public Op {
 public:
  FilterByShape(
      const std::string& key,
      int dim,
      int64_t low = -1,
      int64_t high = -1);

  virtual Sample apply(const Sample& sample) const override;

 private:
  std::string key_;
  int dim_;
  int64_t low_;
  int64_t high_;
};

} // namespace op
} // namespace data
} // namespace mlx
