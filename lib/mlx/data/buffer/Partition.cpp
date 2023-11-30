// Copyright © 2023 Apple Inc.

#include "mlx/data/buffer/Partition.h"

namespace mlx {
namespace data {
namespace buffer {

Partition::Partition(
    std::shared_ptr<Buffer> buffer,
    int64_t num_partitions,
    int64_t partition)
    : buffer_(buffer), numPartitions_(num_partitions), partition_(partition) {
  if (num_partitions < 0) {
    throw std::runtime_error(
        "Partition: number of partitions must be positive");
  }
  if (partition < 0 || partition >= num_partitions) {
    throw std::runtime_error("Partition: selected partition is out of range");
  }
  size_ = buffer->size() / num_partitions;
  if (partition_ < (buffer->size() % num_partitions)) {
    size_++;
  }
}

Sample Partition::get(int64_t idx) const {
  if (idx < 0 || idx > size_) {
    throw std::runtime_error("Partition: index out of range");
  }
  return buffer_->get(idx * numPartitions_ + partition_);
}

int64_t Partition::size() const {
  return size_;
}

} // namespace buffer
} // namespace data
} // namespace mlx
