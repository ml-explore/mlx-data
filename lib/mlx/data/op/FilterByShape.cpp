// Copyright © 2023 Apple Inc.

#include "mlx/data/op/FilterByShape.h"

namespace mlx {
namespace data {
namespace op {
FilterByShape::FilterByShape(
    const std::string& key,
    int dim,
    int64_t low,
    int64_t high)
    : key_(key), dim_(dim), low_(low), high_(high) {}
Sample FilterByShape::apply(const Sample& sample) const {
  auto array = sample::check_key(sample, key_, ArrayType::Any);
  auto dim = dim_;
  if (dim < 0) {
    dim += array->ndim();
  }
  if (dim < 0 || dim >= array->ndim()) {
    return Sample();
  }
  if ((low_ >= 0) && (array->shape(dim) < low_)) {
    return Sample();
  }
  if ((high_ >= 0) && (array->shape(dim) > high_)) {
    return Sample();
  }
  return sample;
}
} // namespace op
} // namespace data
} // namespace mlx
