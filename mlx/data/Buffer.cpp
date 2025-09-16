// Copyright © 2023 Apple Inc.

#include "mlx/data/Buffer.h"
#include "mlx/data/Stream.h"
#include "mlx/data/buffer/Append.h"
#include "mlx/data/buffer/Batch.h"
#include "mlx/data/buffer/DynamicBatch.h"
#include "mlx/data/buffer/FilesFromTAR.h"
#include "mlx/data/buffer/FromVector.h"
#include "mlx/data/buffer/Partition.h"
#include "mlx/data/buffer/Perm.h"
#include "mlx/data/buffer/Shuffle.h"
#include "mlx/data/stream/FromBuffer.h"
#include "mlx/data/stream/OrderedPrefetch.h"

namespace mlx {
namespace data {

Buffer::Buffer(const std::shared_ptr<buffer::Buffer>& self)
    : Dataset<Buffer, buffer::Buffer>(self) {};

Sample Buffer::get(int64_t idx) const {
  return self_->get(idx);
}
int64_t Buffer::size() const {
  return self_->size();
}

Buffer Buffer::batch(
    int64_t batch_size,
    const std::unordered_map<std::string, double>& pad_values,
    const std::unordered_map<std::string, int>& batch_dims) const {
  return Buffer(std::make_shared<buffer::Batch>(
      self_, batch_size, pad_values, batch_dims));
}

Buffer Buffer::batch(
    const std::vector<int64_t>& batch_sizes,
    const std::unordered_map<std::string, double>& pad_values,
    const std::unordered_map<std::string, int>& batch_dims) const {
  return Buffer(std::make_shared<buffer::Batch>(
      self_, batch_sizes, pad_values, batch_dims));
}

Buffer Buffer::dynamic_batch(
    const std::string& key,
    int64_t max_data_size,
    const std::unordered_map<std::string, double>& pad_values,
    const std::unordered_map<std::string, int>& batch_dims) const {
  return Buffer(std::make_shared<buffer::DynamicBatch>(
      self_, key, max_data_size, pad_values, batch_dims));
}

Buffer Buffer::dynamic_batch(
    const Buffer& size_buffer,
    const std::string& key,
    int64_t max_data_size,
    const std::unordered_map<std::string, double>& pad_values,
    const std::unordered_map<std::string, int>& batch_dims) const {
  return Buffer(std::make_shared<buffer::DynamicBatch>(
      self_, size_buffer.self_, key, max_data_size, pad_values, batch_dims));
}

Stream Buffer::ordered_prefetch(int prefetch_size, int num_thread) const {
  return Stream(std::make_shared<stream::OrderedPrefetch>(
      self_, prefetch_size, num_thread));
}

Buffer Buffer::partition(int64_t num_partitions, int64_t partition) const {
  return Buffer(
      std::make_shared<buffer::Partition>(self_, num_partitions, partition));
}
Buffer Buffer::partition_if(
    bool cond,
    int64_t num_partitions,
    int64_t partition) const {
  if (cond) {
    return this->partition(num_partitions, partition);
  } else {
    return Buffer(self_);
  }
}

Buffer Buffer::append(const Buffer& buffer) {
  return Buffer(std::make_shared<buffer::Append>(self_, buffer.self_));
}

Buffer Buffer::perm(const std::vector<int64_t>& perm) {
  return Buffer(std::make_shared<buffer::Perm>(self_, perm));
}

Buffer Buffer::shuffle() {
  return Buffer(std::make_shared<buffer::Shuffle>(self_));
}
Buffer Buffer::shuffle_if(bool cond) {
  if (cond) {
    return shuffle();
  } else {
    return Buffer(self_);
  }
}

Stream Buffer::to_stream() {
  return Stream(std::make_shared<stream::FromBuffer>(self_));
}

Buffer Buffer::concretize() {
  return Buffer(std::make_shared<buffer::FromVector>(self_));
}

// Buffer buffered(
//     int64_t bufferSize,
//     std::function<BufferInterface(const BufferInterface)>
//     onRefill = onRefillDefault) const;

Buffer buffer_from_vector(const std::vector<Sample>& data) {
  return Buffer(std::make_shared<buffer::FromVector>(data));
}
Buffer buffer_from_vector(std::vector<Sample>&& data) {
  return Buffer(std::make_shared<buffer::FromVector>(data));
}
Buffer
files_from_tar(const std::string& tarfile, bool nested, int num_threads) {
  return Buffer(
      std::make_shared<buffer::FilesFromTAR>(tarfile, nested, num_threads));
}

} // namespace data
} // namespace mlx
