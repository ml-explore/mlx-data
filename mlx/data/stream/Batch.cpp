// Copyright © 2023 Apple Inc.

#include "mlx/data/stream/Batch.h"
#include <functional>
#include "mlx/data/core/ThreadPool.h"
#include "mlx/data/core/Utils.h"

namespace mlx {
namespace data {
namespace stream {
Batch::Batch(
    const std::shared_ptr<Stream>& stream,
    int64_t batch_size,
    const std::unordered_map<std::string, double>& pad_values,
    const std::unordered_map<std::string, int>& batch_dims)
    : stream_(stream),
      batchSize_(batch_size),
      padValues_(pad_values),
      batchDims_(batch_dims) {
  if (batch_size <= 0) {
    throw std::runtime_error("Batch: batch size must be positive");
  }
}

Sample Batch::next() const {
  std::vector<Sample> samples;
  for (int i = 0; i < batchSize_; i++) {
    auto sample = stream_->next();
    if (sample.empty()) {
      break;
    }
    samples.push_back(std::move(sample));
  }
  if (samples.empty()) {
    return Sample();
  } else {
    return core::merge_batch(samples, padValues_, batchDims_);
  }
}

void Batch::reset() {
  stream_->reset();
}

} // namespace stream
} // namespace data
} // namespace mlx
