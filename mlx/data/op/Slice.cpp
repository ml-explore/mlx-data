// Copyright © 2025 Apple Inc.

#include <random>

#include "mlx/data/core/State.h"
#include "mlx/data/op/Slice.h"

namespace mlx {
namespace data {
namespace op {

Slice::Slice(
    const std::string& ikey,
    int dim,
    int64_t start,
    int64_t end,
    const std::string& okey)
    : Slice(ikey, std::vector<int>{dim}, {start}, {end}, okey) {}

Slice::Slice(
    const std::string& ikey,
    std::vector<int> dims,
    std::vector<int64_t> starts,
    std::vector<int64_t> ends,
    const std::string& okey)
    : KeyTransformOp(ikey, okey),
      dims_(std::move(dims)),
      starts_(std::move(starts)),
      ends_(std::move(ends)) {
  if (dims_.size() != starts_.size() || dims_.size() != ends_.size()) {
    throw std::invalid_argument(
        "Slice: number of dims much match provided starts and ends");
  }
}

std::shared_ptr<Array> Slice::apply_key(
    const std::shared_ptr<const Array>& src) const {
  std::vector<int64_t> offsets(src->ndim(), 0);
  std::vector<int64_t> shape = src->shape();

  for (int i = 0; i < dims_.size(); i++) {
    auto d = src->checkdim(dims_[i]);
    offsets[d] = starts_[i];
    shape[d] = std::min(shape[d] - starts_[i], ends_[i] - starts_[i]);
  }
  return array::sub(src, std::move(offsets), std::move(shape));
}

RandomSlice::RandomSlice(
    const std::string& ikey,
    int dim,
    int64_t size,
    const std::string& okey)
    : RandomSlice(ikey, std::vector<int>{dim}, {size}, okey) {}

RandomSlice::RandomSlice(
    const std::string& ikey,
    std::vector<int> dims,
    std::vector<int64_t> sizes,
    const std::string& okey)
    : KeyTransformOp(ikey, okey),
      dims_(std::move(dims)),
      sizes_(std::move(sizes)) {
  if (dims_.size() != sizes_.size()) {
    throw std::invalid_argument(
        "RandomSlice: number of dims much match provided sizes");
  }
}

std::shared_ptr<Array> RandomSlice::apply_key(
    const std::shared_ptr<const Array>& src) const {
  std::uniform_real_distribution<> rand_offset(0.0, 1.0);
  std::vector<int64_t> offsets(src->ndim(), 0);
  std::vector<int64_t> shape = src->shape();

  auto state = core::get_state();
  for (int i = 0; i < dims_.size(); i++) {
    auto d = src->checkdim(dims_[i]);
    auto max_offset = shape[d] - sizes_[i];
    if (max_offset > 0) {
      shape[d] = sizes_[i];
      offsets[d] = (max_offset + 1) * rand_offset(state->randomGenerator);
    }
  }
  return array::sub(src, std::move(offsets), std::move(shape));
}

} // namespace op
} // namespace data
} // namespace mlx
