// Copyright © 2023 Apple Inc.

#include <stdexcept>

#include "mlx/data/buffer/Buffer.h"

namespace mlx {
namespace data {
namespace buffer {

Sample Buffer::get(const int64_t idx) const {
  throw std::runtime_error("Buffer::get() NYI");
}

int64_t Buffer::size() const {
  throw std::runtime_error("Buffer::size() NYI");
}

Buffer::~Buffer() {}

} // namespace buffer
} // namespace data
} // namespace mlx
