#include "mlx/data/stream/FromBuffer.h"

namespace mlx {
namespace data {
namespace stream {

FromBuffer::FromBuffer(const std::shared_ptr<buffer::Buffer>& buffer)
    : buffer_(buffer), currentIdx_(0) {}

Sample FromBuffer::next() const {
  int64_t idx = -1;
  {
    std::lock_guard<std::mutex> lock(mutex_);
    if (currentIdx_ < buffer_->size()) {
      idx = currentIdx_++;
    }
  }
  if (idx < 0) {
    return Sample();
  } else {
    return buffer_->get(idx);
  }
}

void FromBuffer::reset() {
  std::lock_guard<std::mutex> lock(mutex_);
  currentIdx_ = 0;
}

} // namespace stream
} // namespace data
} // namespace mlx
