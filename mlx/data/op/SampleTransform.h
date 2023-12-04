// Copyright © 2023 Apple Inc.

#pragma once

#include <functional>

#include "mlx/data/op/Op.h"

namespace mlx {
namespace data {
namespace op {

class SampleTransform : public Op {
 public:
  SampleTransform(std::function<Sample(const Sample&)> op);

  virtual Sample apply(const Sample& sample) const;

 private:
  std::function<Sample(const Sample&)> op_;
};

} // namespace op
} // namespace data
} // namespace mlx
