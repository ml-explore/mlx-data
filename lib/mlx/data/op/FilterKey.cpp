// Copyright © 2023 Apple Inc.

#include "mlx/data/op/FilterKey.h"

namespace mlx {
namespace data {
namespace op {
FilterKey::FilterKey(const std::string& key, bool remove)
    : keys_({key}), remove_(remove) {}
FilterKey::FilterKey(const std::vector<std::string>& keys, bool remove)
    : keys_(keys), remove_(remove) {}
Sample FilterKey::apply(const Sample& sample) const {
  Sample res;
  if (remove_) {
    res = sample;
    for (auto& key : keys_) {
      sample::check_key(sample, key, ArrayType::Any);
      res.erase(key);
    }
  } else {
    for (auto& key : keys_) {
      auto array = sample::check_key(sample, key, ArrayType::Any);
      res[key] = array;
    }
  }
  return res;
}
} // namespace op
} // namespace data
} // namespace mlx
