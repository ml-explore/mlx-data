// Copyright © 2023 Apple Inc.

#include "mlx/data/buffer/FromVector.h"

namespace mlx {
namespace data {
namespace buffer {

FromVector::FromVector(const std::vector<Sample>& data) : buffer_(data) {
  check_samples_();
}

FromVector::FromVector(std::vector<Sample>&& data) : buffer_(std::move(data)) {
  check_samples_();
}

Sample FromVector::get(int64_t idx) const {
  if (idx < 0 || idx >= buffer_.size()) {
    throw std::runtime_error("FromVector: index out of range");
  }
  return buffer_[idx];
}

int64_t FromVector::size() const {
  return buffer_.size();
}

void FromVector::check_samples_() const {
  for (auto& sample : buffer_) {
    if (sample.empty()) {
      throw std::runtime_error("FromVector: unexpected empty sample");
    }
  }
}

} // namespace buffer
} // namespace data
} // namespace mlx
