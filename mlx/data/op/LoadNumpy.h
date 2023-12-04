// Copyright © 2023 Apple Inc.

#pragma once

#include "mlx/data/op/KeyTransform.h"

namespace mlx {
namespace data {
namespace op {

class LoadNumpy : public KeyTransformOp {
 public:
  LoadNumpy(
      const std::string& ikey,
      const std::string& prefix = "",
      bool from_memory = false,
      const std::string& okey = "");

  virtual std::shared_ptr<Array> apply_key(
      const std::shared_ptr<const Array>& src) const override;

 private:
  std::string prefix_;
  bool from_memory_;
};

} // namespace op
} // namespace data
} // namespace mlx
