// Copyright © 2023 Apple Inc.

#pragma once

#include "mlx/data/op/Op.h"

namespace mlx {
namespace data {
namespace op {

class RemoveValue : public Op {
 public:
  RemoveValue(
      const std::string& key,
      const std::string& size_key,
      int dim,
      double value,
      double pad);

  virtual Sample apply(const Sample& sample) const override;

 private:
  std::string key_;
  std::string size_key_;
  int dim_;
  double value_;
  double pad_;
};

} // namespace op
} // namespace data
} // namespace mlx
