// Copyright © 2023 Apple Inc.

#include "mlx/data/stream/DynamicBatch.h"
#include "mlx/data/buffer/Append.h"
#include "mlx/data/buffer/DynamicBatch.h"
#include "mlx/data/buffer/Shuffle.h"

namespace mlx {
namespace data {
namespace stream {

DynamicBatch::DynamicBatch(
    std::shared_ptr<Stream> stream,
    int64_t buffer_size,
    const std::string& key,
    int64_t min_data_size,
    int64_t max_data_size,
    const std::unordered_map<std::string, double>& pad_values,
    const std::unordered_map<std::string, int>& batch_dims,
    bool shuffle,
    bool drop_outliers,
    int64_t max_skipped_samples,
    int num_thread)
    : Buffered(stream, buffer_size, num_thread),
      key_(key),
      min_data_size_(min_data_size),
      max_data_size_(max_data_size),
      pad_values_(pad_values),
      batch_dims_(batch_dims),
      shuffle_(shuffle),
      drop_outliers_(drop_outliers),
      max_skipped_samples_(max_skipped_samples) {};

std::shared_ptr<buffer::Buffer> DynamicBatch::on_refill(
    const std::shared_ptr<buffer::Buffer>& buffer) const {
  std::shared_ptr<buffer::Buffer> new_buffer = buffer;
  if (skipped_samples_buffer_) {
    new_buffer =
        std::make_shared<buffer::Append>(new_buffer, skipped_samples_buffer_);
  }
  auto dyn_buffer = std::make_shared<buffer::DynamicBatch>(
      new_buffer,
      key_,
      min_data_size_,
      max_data_size_,
      pad_values_,
      batch_dims_,
      drop_outliers_);
  auto skipped_samples = dyn_buffer->skipped_samples();
  if (skipped_samples.size() > 0) {
    if ((max_skipped_samples_ > 0) &&
        (skipped_samples.size() > max_skipped_samples_)) {
      skipped_samples.resize(max_skipped_samples_);
    }
    skipped_samples_buffer_ =
        std::make_shared<buffer::Perm>(new_buffer, skipped_samples);
    // DEBUG: concretize
  } else {
    skipped_samples_buffer_ = nullptr;
  }
  new_buffer = dyn_buffer;
  if (shuffle_) {
    new_buffer = std::make_shared<buffer::Shuffle>(new_buffer);
  }
  return new_buffer;
};

DynamicBatch::~DynamicBatch() {
  finish_background_tasks();
}

} // namespace stream
} // namespace data
} // namespace mlx
