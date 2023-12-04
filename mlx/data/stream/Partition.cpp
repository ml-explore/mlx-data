// Copyright © 2023 Apple Inc.

#include "mlx/data/stream/Partition.h"

namespace mlx {
namespace data {
namespace stream {

Partition::Partition(
    const std::shared_ptr<Stream>& stream,
    int64_t num_partitions,
    int64_t partition)
    : stream_(stream), numPartitions_(num_partitions), partition_(partition) {
  if (num_partitions < 0) {
    throw std::runtime_error(
        "Partition: number of partitions must be positive");
  }
  if (partition < 0 || partition >= num_partitions) {
    throw std::runtime_error("Partition: selected partition is out of range");
  }
}

Sample Partition::next() const {
  std::unique_lock lock(stream_mutex_);

  Sample res;
  for (int i = 0; i < numPartitions_; i++) {
    auto sample = stream_->next();
    if (i == partition_) {
      res = std::move(sample);
    }
  }

  return res;
}

void Partition::reset() {
  std::unique_lock lock(stream_mutex_);
  stream_->reset();
}

} // namespace stream
} // namespace data
} // namespace mlx
