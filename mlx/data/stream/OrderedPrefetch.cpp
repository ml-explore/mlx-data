// Copyright © 2023 Apple Inc.

#include "mlx/data/stream/OrderedPrefetch.h"

namespace mlx {
namespace data {
namespace stream {

OrderedPrefetch::OrderedPrefetch(
    const std::shared_ptr<buffer::Buffer>& buffer,
    int prefetch_size,
    int num_thread)
    : buffer_(buffer),
      pool_(std::make_shared<core::ThreadPool>(num_thread)),
      prefetchSize_(prefetch_size),
      currentIdx_(0) {
  if (prefetchSize_ <= 0) {
    throw std::runtime_error(
        "Prefetch: prefetch size must be strictly positive");
  }
}

OrderedPrefetch::~OrderedPrefetch() {
  std::lock_guard<std::mutex> lock(mutex_);

  prefetchCache_.clear();
}

Sample OrderedPrefetch::next() const {
  std::unique_lock<std::mutex> lock(mutex_);

  // First time we are called so enqueue all the fetching
  if (prefetchCache_.size() < prefetchSize_) {
    for (int i = 0; i < std::min(prefetchSize_, buffer_->size()); i++) {
      prefetchCache_.emplace_back(
          pool_->enqueue([b = buffer_, i] { return b->get(i); }));
    }
  }

  int64_t idx = -1;
  if (currentIdx_ < buffer_->size()) {
    idx = currentIdx_++;
  }

  if (idx < 0) {
    return Sample();
  } else {
    int f_idx = idx % prefetchSize_;
    std::future<Sample> fsample(std::move(prefetchCache_[f_idx]));
    int next_idx = idx + prefetchSize_;
    if (next_idx < buffer_->size()) {
      prefetchCache_[f_idx] =
          pool_->enqueue([b = buffer_, next_idx] { return b->get(next_idx); });
    }
    lock.unlock();
    return fsample.get();
  }
}

void OrderedPrefetch::reset() {
  std::lock_guard<std::mutex> lock(mutex_);
  currentIdx_ = 0;

  prefetchCache_.clear();
}

} // namespace stream
} // namespace data
} // namespace mlx
