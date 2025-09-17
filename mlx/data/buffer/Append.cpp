// Copyright © 2023 Apple Inc.

#include "mlx/data/buffer/Append.h"

namespace mlx {
namespace data {
namespace buffer {

Append::Append(
    const std::shared_ptr<Buffer>& buffer1,
    const std::shared_ptr<Buffer>& buffer2)
    : buffer1_(buffer1), buffer2_(buffer2) {}

Sample Append::get(int64_t idx) const {
  int64_t size1 = buffer1_->size();
  int64_t size2 = buffer2_->size();

  if (idx < 0 || (idx > (size1 + size2))) {
    throw std::runtime_error("Append: index out of range");
  }

  if (idx < size1) {
    return buffer1_->get(idx);
  } else {
    return buffer2_->get(idx - size1);
  }
}

int64_t Append::size() const {
  return buffer1_->size() + buffer2_->size();
}

} // namespace buffer
} // namespace data
} // namespace mlx
