// Copyright © 2023 Apple Inc.

#include "mlx/data/op/Pad.h"

namespace mlx {
namespace data {
namespace op {

Pad::Pad(
    const std::string& ikey,
    int dim,
    int64_t lpad,
    int64_t rpad,
    double value,
    const std::string& okey)
    : KeyTransformOp(ikey, okey),
      dim_(dim),
      lpad_(lpad),
      rpad_(rpad),
      value_(value) {
  if (lpad_ < 0 || rpad_ < 0) {
    throw std::runtime_error("Pad: pad value must be positive");
  }
}
std::shared_ptr<Array> Pad::apply_key(
    const std::shared_ptr<const Array>& src) const {
  auto dim = src->checkdim(dim_);
  return array::pad(src, dim, lpad_, rpad_, value_);
}

PadToSize::PadToSize(
    const std::string& ikey,
    int dim,
    int64_t size,
    double value,
    const std::string& okey)
    : KeyTransformOp(ikey, okey), dim_(dim), sizes_({size}), value_(value) {}
PadToSize::PadToSize(
    const std::string& ikey,
    int dim,
    const std::vector<int64_t>& sizes,
    double value,
    const std::string& okey)
    : KeyTransformOp(ikey, okey), dim_(dim), sizes_(sizes), value_(value) {}
std::shared_ptr<Array> PadToSize::apply_key(
    const std::shared_ptr<const Array>& src) const {
  auto dim = src->checkdim(dim_);
  int64_t min_diff_idx = -1;
  int64_t min_diff_size = std::numeric_limits<int64_t>::max();
  int64_t dim_size = src->shape(dim);
  for (int i = 0; i < sizes_.size(); i++) {
    auto diff_size = sizes_[i] - dim_size;
    if (diff_size > 0 && diff_size < min_diff_size) {
      min_diff_size = diff_size;
      min_diff_idx = i;
    }
  }
  if (min_diff_idx >= 0) {
    return array::pad(src, dim, 0, min_diff_size, value_);
  } else {
    return mlx::data::array::clone(src);
  }
}

PadToMultiple::PadToMultiple(
    const std::string& ikey,
    int dim,
    int64_t size,
    double value,
    const std::string& okey)
    : KeyTransformOp(ikey, okey), dim_(dim), size_(size), value_(value) {}
std::shared_ptr<Array> PadToMultiple::apply_key(
    const std::shared_ptr<const Array>& src) const {
  auto dim = src->checkdim(dim_);
  int64_t mod = src->shape(dim) % size_;
  if (mod != 0) {
    return array::pad(src, dim, 0, size_ - mod, value_);
  } else {
    return mlx::data::array::clone(src);
  }
}

} // namespace op
} // namespace data
} // namespace mlx
