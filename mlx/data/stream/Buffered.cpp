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
    int num_thread)
    : stream_(stream),
      buffer_size_(buffer_size),
      pool_(std::make_shared<core::ThreadPool>(num_thread + 1)),
      current_index_(0),
      buffer_(nullptr) {}

std::future<std::shared_ptr<buffer::Buffer>>
Buffered::background_buffer_fetch_() const {
  return pool_->enqueue([this]() -> std::shared_ptr<buffer::Buffer> {
    std::vector<std::future<Sample>> future_buffer;
    for (int i = 0; pool_is_alive_ && (i < buffer_size_); i++) {
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

    if (pool_is_alive_)
      // note: only one on_refill() will run at the same time
      // as background_buffer_fetch_() call is guarded by a mutex
      return on_refill(std::make_shared<FromVector>(buffer));
    else {
      return nullptr;
    }
  });
}

Sample Buffered::next() const {
  std::unique_lock lock(mutex_);

  // First run
  if (buffer_ == nullptr) {
    buffer_ = background_buffer_fetch_().get();
    next_buffer_ = background_buffer_fetch_();
  }

  // We are done
  if (buffer_->size() == 0) {
    return Sample();
  }

  // Normal running
  if (current_index_ >= buffer_->size()) {
    current_index_ = 0;
    buffer_ = next_buffer_.get();
    next_buffer_ = background_buffer_fetch_();

    if (buffer_->size() == 0) {
      return Sample();
    }
  }

  return buffer_->get(current_index_++);
}

void Buffered::reset() {
  std::unique_lock lock(mutex_);

  buffer_ = nullptr;
  if (next_buffer_.valid()) {
    next_buffer_.get();
  }
  stream_->reset();
}

std::shared_ptr<buffer::Buffer> Buffered::on_refill(
    const std::shared_ptr<buffer::Buffer>& buffer) const {
  return buffer;
}

void Buffered::finish_background_tasks() {
  pool_is_alive_ = false;
  pool_ = nullptr;
}

Buffered::~Buffered() {
  finish_background_tasks();
}

CallbackBuffered::CallbackBuffered(
    const std::shared_ptr<Stream>& stream,
    int64_t buffer_size,
    std::function<std::shared_ptr<buffer::Buffer>(
        const std::shared_ptr<buffer::Buffer>&)> on_refill,
    int num_thread)
    : Buffered(stream, buffer_size, num_thread), on_refill_(on_refill) {};

std::shared_ptr<buffer::Buffer> CallbackBuffered::on_refill(
    const std::shared_ptr<buffer::Buffer>& buffer) const {
  return on_refill_(buffer);
}

} // namespace stream
} // namespace data
} // namespace mlx
