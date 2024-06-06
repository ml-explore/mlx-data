// Copyright © 2024 Apple Inc.

#include "mlx/data/op/Replace.h"
#include "mlx/data/core/Utils.h"

namespace mlx {
namespace data {
namespace op {

Replace::Replace(
    const std::string& key,
    const std::string& old,
    const std::string& replacement,
    int count)
    : key_(key),
      old_(std::make_shared<Array>(old)),
      replacement_(std::make_shared<Array>(replacement)),
      count_(count) {}

Sample Replace::apply(const Sample& sample) const {
  auto value = sample::check_key(sample, key_, old_->type());
  value = core::replace(value, old_, replacement_, count_);
  auto new_sample = sample;
  new_sample[key_] = value;
  return new_sample;
}

} // namespace op
} // namespace data
} // namespace mlx
