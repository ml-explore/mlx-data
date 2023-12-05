// Copyright © 2023 Apple Inc.

#pragma once

#include "mlx/data/stream/Stream.h"

namespace mlx {
namespace data {
namespace stream {

class Partition : public Stream {
 public:
  Partition(
      const std::shared_ptr<Stream>& stream,
      int64_t num_partitions,
      int64_t partition);

  virtual Sample next() const override;
  virtual void reset() override;

 private:
  std::shared_ptr<Stream> stream_;
  int64_t numPartitions_;
  int64_t partition_;
  mutable std::mutex stream_mutex_;
};

} // namespace stream
} // namespace data
} // namespace mlx
