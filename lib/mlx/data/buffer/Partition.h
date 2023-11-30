// Copyright © 2023 Apple Inc.

#pragma once

#include "mlx/data/buffer/Buffer.h"

namespace mlx {
namespace data {
namespace buffer {

class Partition : public Buffer {
 public:
  Partition(
      std::shared_ptr<Buffer> buffer,
      int64_t num_partitions,
      int64_t partition);

  virtual Sample get(int64_t idx) const override;
  virtual int64_t size() const override;

 private:
  std::shared_ptr<Buffer> buffer_;
  int64_t numPartitions_;
  int64_t partition_;
  int64_t size_;
};

} // namespace buffer
} // namespace data
} // namespace mlx
