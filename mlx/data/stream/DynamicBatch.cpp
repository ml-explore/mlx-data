// Copyright © 2023 Apple Inc.

#include "mlx/data/stream/DynamicBatch.h"
#include "mlx/data/buffer/DynamicBatch.h"
#include "mlx/data/buffer/Shuffle.h"

namespace mlx {
namespace data {
namespace stream {

DynamicBatch::DynamicBatch(
    std::shared_ptr<Stream> stream,
    int64_t buffer_size,
    const std::string& key,
    int64_t max_data_size,
    const std::unordered_map<std::string, double>& pad_values,
    const std::unordered_map<std::string, int>& batch_dims,
    bool shuffle,
    int num_thread)
    : Buffered(stream, buffer_size, num_thread),
      key_(key),
      max_data_size_(max_data_size),
      pad_values_(pad_values),
      batch_dims_(batch_dims),
      shuffle_(shuffle) {};

std::shared_ptr<buffer::Buffer> DynamicBatch::on_refill(
    const std::shared_ptr<buffer::Buffer>& buffer) const {
  std::shared_ptr<buffer::Buffer> dyn_buffer =
      std::make_shared<buffer::DynamicBatch>(
          buffer, key_, max_data_size_, pad_values_, batch_dims_);
  if (shuffle_) {
    dyn_buffer = std::make_shared<buffer::Shuffle>(dyn_buffer);
  }
  return dyn_buffer;
};

} // namespace stream
} // namespace data
} // namespace mlx
