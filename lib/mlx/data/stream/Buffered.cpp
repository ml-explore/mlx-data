// Copyright © 2023 Apple Inc.

#include "mlx/data/stream/Buffered.h"
#include "mlx/data/buffer/FromVector.h"
#include "mlx/data/stream/FromBuffer.h"

namespace mlx {
namespace data {
namespace stream {

using FromVector = mlx::data::buffer::FromVector;

Buffered::Buffered(
    const std::shared_ptr<Stream>& stream,
    int64_t buffer_size,
    std::function<std::shared_ptr<buffer::Buffer>(
        const std::shared_ptr<buffer::Buffer>&)> on_refill,
    int num_thread)
    : stream_(stream),
      bufferSize_(buffer_size),
      onRefill_(on_refill),
      pool_(std::make_shared<core::ThreadPool>(num_thread + 1)),
      currentIndex_(0),
      buffer_(nullptr) {}

std::future<std::shared_ptr<buffer::Buffer>>
Buffered::background_buffer_fetch_() const {
  return pool_->enqueue([this]() -> std::shared_ptr<buffer::Buffer> {
    std::vector<std::future<Sample>> future_buffer;
    for (int i = 0; i < bufferSize_; i++) {
      future_buffer.push_back(
          pool_->enqueue([this] { return stream_->next(); }));
    }
    std::vector<Sample> buffer;
    for (auto& fsample : future_buffer) {
      Sample sample = fsample.get();
      if (!sample.empty()) {
        buffer.push_back(sample);
      }
    }

    return onRefill_(std::make_shared<FromVector>(buffer));
  });
}

Sample Buffered::next() const {
  std::unique_lock lock(mutex_);

  // First run
  if (buffer_ == nullptr) {
    buffer_ = background_buffer_fetch_().get();
    nextBuffer_ = background_buffer_fetch_();
  }

  // We are done
  if (buffer_->size() == 0) {
    return Sample();
  }

  // Normal running
  if (currentIndex_ >= buffer_->size()) {
    currentIndex_ = 0;
    buffer_ = nextBuffer_.get();
    nextBuffer_ = background_buffer_fetch_();

    if (buffer_->size() == 0) {
      return Sample();
    }
  }

  return buffer_->get(currentIndex_++);
}

void Buffered::reset() {
  std::unique_lock lock(mutex_);

  buffer_ = nullptr;
  if (nextBuffer_.valid()) {
    nextBuffer_.get();
  }
  stream_->reset();
}

std::shared_ptr<buffer::Buffer> Buffered::on_refill_default(
    const std::shared_ptr<buffer::Buffer>& buffer) {
  return buffer;
}

} // namespace stream
} // namespace data
} // namespace mlx
