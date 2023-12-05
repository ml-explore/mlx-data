// Copyright © 2023 Apple Inc.

#include <random>

#include "mlx/data/core/State.h"
#include "mlx/data/stream/Shuffle.h"

namespace mlx {
namespace data {
namespace stream {

Shuffle::Shuffle(const std::shared_ptr<Stream>& stream, int buffer_size)
    : stream_(stream), buffer_size_(buffer_size) {}

Sample Shuffle::next() const {
  // The while is really only for case 1 below but it reads a bit better than
  // putting the while loop in lines 30-35 I believe.
  while (true) {
    // First get a sample from the underlying stream
    auto sample = stream_->next();

    // Now there are a couple of cases we have to consider
    //
    // 1. The sample is not empty and the buffer is not full -> keep fetching
    // 2. The sample is not empty and the buffer is full -> standard case
    // 3. The sample is empty and the buffer is not empty -> pop a random sample
    // 4. The sample is empty and the buffer is empty -> we are done

    if (!sample.empty()) {
      std::uniform_int_distribution<int> pos_dis(0, buffer_size_ - 1);
      int pos = pos_dis(core::get_state()->randomGenerator);

      {
        std::unique_lock lock(mutex_);

        if (buffer_.size() < buffer_size_) {
          buffer_.emplace_back(sample);
          continue;
        }

        std::swap(sample, buffer_[pos]);

        return sample;
      }
    } else {
      std::unique_lock lock(mutex_);

      if (buffer_.size() > 0) {
        std::uniform_int_distribution<int> pos_dis(0, buffer_.size() - 1);
        int pos = pos_dis(core::get_state()->randomGenerator);

        sample = std::move(buffer_[pos]);
        buffer_.erase(buffer_.begin() + pos);
      }

      return sample;
    }
  }
}

void Shuffle::reset() {
  std::unique_lock lock(mutex_);

  stream_->reset();
  buffer_.clear();
}

} // namespace stream
} // namespace data
} // namespace mlx
