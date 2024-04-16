// Copyright © 2024 Apple Inc.

#pragma once

#include "mlx/data/op/Op.h"

namespace mlx {
namespace data {
namespace op {

class Replace : public Op {
 public:
  Replace(
      const std::string& key,
      const std::string& old,
      const std::string& replacement,
      int count);

  virtual Sample apply(const Sample& sample) const override;

 private:
  std::string key_;
  std::shared_ptr<Array> old_;
  std::shared_ptr<Array> replacement_;
  int count_;
};

} // namespace op
} // namespace data
} // namespace mlx
