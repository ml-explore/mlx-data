// Copyright © 2023 Apple Inc.

#include <stdexcept>

#include "mlx/data/op/KeyTransform.h"

namespace mlx {
namespace data {
namespace op {

KeyTransformOp::KeyTransformOp(const std::string& ikey, const std::string& okey)
    : ikey_(ikey), okey_(okey) {};

Sample KeyTransformOp::apply(const Sample& sample) const {
  auto src = sample::check_key(sample, ikey_, ArrayType::Any);
  auto dst = apply_key(src);
  auto res = sample;
  auto okey = (okey_.empty() ? ikey_ : okey_);
  res[okey] = dst;
  return res;
}

KeyTransform::KeyTransform(
    const std::string& ikey,
    std::function<std::shared_ptr<Array>(const std::shared_ptr<const Array>&)>
        op,
    const std::string& okey)
    : KeyTransformOp(ikey, okey), op_(op) {};

std::shared_ptr<Array> KeyTransform::apply_key(
    const std::shared_ptr<const Array>& x) const {
  return op_(x);
}

} // namespace op
} // namespace data
} // namespace mlx
