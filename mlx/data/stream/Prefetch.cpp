// Copyright © 2023 Apple Inc.

#include "mlx/data/stream/Prefetch.h"

namespace mlx {
namespace data {
namespace stream {

Prefetch::Prefetch(
    const std::shared_ptr<Stream>& stream,
    int prefetch_size,
    int num_thread)
    : stream_(stream),
      pool_(std::make_shared<core::ThreadPool>(num_thread)),
      prefetchSize_(prefetch_size) {
  if (prefetchSize_ < 0) {
    throw std::runtime_error("Prefetch: prefetch size must be positive");
  }
}

Prefetch::~Prefetch() {
  std::unique_lock lock(mutex_);
  while (prefetchCache_.size()) {
    prefetchCache_.front().get();
    prefetchCache_.pop();
  }
}

Sample Prefetch::next() const {
  std::unique_lock lock(mutex_);

  // First time we are called so enqueue all the fetching
  if (prefetchCache_.size() < prefetchSize_) {
    for (int i = 0; i < prefetchSize_; i++) {
      prefetchCache_.emplace(
          pool_->enqueue([s = stream_] { return s->next(); }));
    }
  }

  // We are looping prefetchSize_ times. If all we get is empty then the
  // underlying stream is indeed exhausted.
  Sample res;
  for (int i = 0; i < prefetchSize_; i++) {
    std::future<Sample> fsample;
    fsample = std::move(prefetchCache_.front());
    prefetchCache_.pop();
    prefetchCache_.emplace(pool_->enqueue([s = stream_] { return s->next(); }));
    res = fsample.get();

    if (!res.empty()) {
      break;
    }
  }

  return res;
}

void Prefetch::reset() {
  std::unique_lock lock(mutex_);

  while (prefetchCache_.size()) {
    prefetchCache_.front().get();
    prefetchCache_.pop();
  }
  stream_->reset();
}

} // namespace stream
} // namespace data
} // namespace mlx
