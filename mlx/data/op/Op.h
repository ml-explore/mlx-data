// Copyright © 2023 Apple Inc.

#pragma once

#include <memory>
#include <string>
#include <unordered_map>

#include "mlx/data/Array.h"
#include "mlx/data/Sample.h"

namespace mlx {
namespace data {
namespace op {

class Op {
 public:
  Op(){};

  // DEBUG: (debatable) sample could be not const
  virtual Sample apply(const Sample& sample) const;

  virtual ~Op();
};

} // namespace op
} // namespace data
} // namespace mlx
