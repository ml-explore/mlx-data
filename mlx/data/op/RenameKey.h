// Copyright © 2023 Apple Inc.

#pragma once

#include "mlx/data/op/Op.h"

namespace mlx {
namespace data {
namespace op {

class RenameKey : public Op {
 public:
  RenameKey(const std::string& ikey, const std::string& okey);

  virtual Sample apply(const Sample& sample) const override;

 private:
  std::string ikey_;
  std::string okey_;
};

} // namespace op
} // namespace data
} // namespace mlx
