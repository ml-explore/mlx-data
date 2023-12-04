// Copyright © 2023 Apple Inc.

#include "mlx/data/op/RenameKey.h"

namespace mlx {
namespace data {
namespace op {
RenameKey::RenameKey(const std::string& ikey, const std::string& okey)
    : ikey_(ikey), okey_(okey) {}
Sample RenameKey::apply(const Sample& sample) const {
  auto input_array = sample::check_key(sample, ikey_, ArrayType::Any);
  if (ikey_ == okey_) {
    return sample;
  }
  auto res = sample;
  res[okey_] = input_array;
  res.erase(ikey_);
  return res;
}
} // namespace op
} // namespace data
} // namespace mlx
