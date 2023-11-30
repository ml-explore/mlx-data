// Copyright © 2023 Apple Inc.

#pragma once

#include "mlx/data/op/Op.h"

namespace mlx {
namespace data {
namespace op {

class Shape : public Op {
 public:
  Shape(const std::string& ikey, int dim, const std::string& okey);
  Shape(const std::string& ikey, const std::string& okey);

  virtual Sample apply(const Sample& sample) const override;

 private:
  std::string ikey_;
  int dim_;
  std::string okey_;
  bool fullShape_;
};

} // namespace op
} // namespace data
} // namespace mlx
