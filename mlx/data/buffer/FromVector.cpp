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
    throw std::out_of_range("FromVector: index out of range");
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

std::shared_ptr<FromVector> FromVector::merge(
    const std::shared_ptr<FromVector>& buffer) const {
  std::vector<Sample> samples = buffer_;
  samples.insert(
      samples.begin(), buffer->buffer_.begin(), buffer->buffer_.end());
  return std::make_shared<FromVector>(std::move(samples));
}

std::shared_ptr<FromVector> FromVector::perm(
    const std::vector<int64_t>& indices) const {
  std::vector<Sample> samples;
  auto n = buffer_.size();
  samples.reserve(indices.size());
  for (int64_t idx : indices) {
    if ((idx >= 0) && (idx < n)) {
      samples.push_back(buffer_[idx]);
    } else {
      throw std::runtime_error("FromVector: index out of bound");
    }
  }
  return std::make_shared<FromVector>(std::move(samples));
}

} // namespace buffer
} // namespace data
} // namespace mlx
