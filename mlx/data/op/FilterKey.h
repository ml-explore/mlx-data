// Copyright © 2023 Apple Inc.

#pragma once

#include "mlx/data/op/Op.h"

namespace mlx {
namespace data {
namespace op {

class FilterKey : public Op {
 public:
  FilterKey(const std::string& key, bool remove = false);
  FilterKey(const std::vector<std::string>& keys, bool remove = false);

  virtual Sample apply(const Sample& sample) const override;

 private:
  std::vector<std::string> keys_;
  bool remove_;
};

} // namespace op
} // namespace data
} // namespace mlx
