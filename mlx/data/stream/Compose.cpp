// Copyright © 2023 Apple Inc.

#include <stdexcept>

#include "mlx/data/stream/Compose.h"

namespace mlx {
namespace data {
namespace stream {

Compose::Compose(
    std::shared_ptr<Stream>& stream,
    std::function<std::shared_ptr<Stream>(const Sample& sample)> op)
    : stream_(stream), op_(op) {};

bool Compose::next_stream_() const {
  auto sample = stream_->next();
  if (sample.empty()) {
    return false;
  }
  composedStream_ = op_(sample);
  if (!composedStream_) {
    throw std::runtime_error(
        "Compose: composer unexpectedly returned a nullptr stream");
  }
  return true;
}

Sample Compose::next() const {
  // note: composedStream_ is read by many threads
  // and written by one thread once in a while
  std::shared_lock slock(mutex_);

  // Composed stream is not created yet
  if (composedStream_ == nullptr) {
    slock.unlock();
    {
      std::unique_lock ulock(mutex_);
      if (!composedStream_) {
        if (!next_stream_()) {
          return Sample(); // EOF
        }
      }
    }
    slock.lock();
  }

  Sample sample;
  while (sample.empty()) {
    sample = composedStream_->next();
    if (sample.empty()) {
      slock.unlock();
      {
        std::unique_lock ulock(mutex_);
        // maybe we got the lock after the stream was updated
        sample = composedStream_->next();
        if (sample.empty()) {
          if (!next_stream_()) {
            return sample; // EOF
          }
          sample = composedStream_->next();
        }
      }
      slock.lock();
    }
  }

  return sample;
}

void Compose::reset() {
  std::unique_lock lock(mutex_);
  stream_->reset();
  composedStream_ = nullptr;
}

} // namespace stream
} // namespace data
} // namespace mlx
