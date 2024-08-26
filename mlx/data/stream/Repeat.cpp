// Copyright © 2023 Apple Inc.

#include <stdexcept>

#include "mlx/data/stream/Repeat.h"

namespace mlx {
namespace data {
namespace stream {

Repeat::Repeat(const std::shared_ptr<Stream>& stream, int64_t num_time)
    : stream_(stream), numTime_(num_time), numDone_(0){};

Sample Repeat::next() const {
  Sample sample;
  {
    std::shared_lock slock(stream_reset_mutex_);
    sample = stream_->next();
  }

  // Empty sample we may need to reset the underlying stream
  if (sample.empty()) {
    {
      std::unique_lock lock(stream_reset_mutex_);

      // Get another sample in case someone else reset the stream in the
      // meantime.
      sample = stream_->next();
      if (!sample.empty()) {
        return sample;
      }

      // We are not allowed to reset anymore
      if (numTime_ > 0 && numDone_ >= numTime_) {
        return sample;
      }

      numDone_++;
      stream_->reset();
      sample = stream_->next();
    }

    return sample;
  }

  return sample;
}

void Repeat::reset() {
  std::unique_lock ulock(stream_reset_mutex_);
  stream_->reset();
  numDone_ = 0;
}

} // namespace stream
} // namespace data
} // namespace mlx
