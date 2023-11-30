// Copyright © 2023 Apple Inc.

#pragma once

#include "mlx/data/buffer/Buffer.h"

namespace mlx {
namespace data {
namespace buffer {

class Batch : public Buffer {
 public:
  Batch(
      const std::shared_ptr<Buffer>& op,
      int64_t batch_size,
      const std::unordered_map<std::string, double>& pad_values = {},
      const std::unordered_map<std::string, int>& batch_dims = {});
  Batch(
      const std::shared_ptr<Buffer>& op,
      const std::vector<int64_t>& batch_sizes,
      const std::unordered_map<std::string, double>& pad_values = {},
      const std::unordered_map<std::string, int>& batch_dims = {});

  virtual Sample get(int64_t idx) const override;
  virtual int64_t size() const override;

 private:
  std::shared_ptr<Buffer> op_;
  int64_t batchSize_;
  std::vector<int64_t> batchOffsets_;
  std::vector<int64_t> batchSizes_;
  std::unordered_map<std::string, double> padValues_;
  std::unordered_map<std::string, int> batchDims_;
  int64_t size_;
};

} // namespace buffer
} // namespace data
} // namespace mlx
