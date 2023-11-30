// Copyright © 2023 Apple Inc.

#pragma once

#include "mlx/data/stream/Stream.h"

namespace mlx {
namespace data {
namespace stream {

class Batch : public Stream {
 public:
  Batch(
      const std::shared_ptr<Stream>& stream,
      int64_t batch_size,
      const std::unordered_map<std::string, double>& pad_values = {},
      const std::unordered_map<std::string, int>& batch_dims = {});
  virtual Sample next() const override;
  virtual void reset() override;

 private:
  std::shared_ptr<Stream> stream_;
  int64_t batchSize_;
  std::unordered_map<std::string, double> padValues_;
  std::unordered_map<std::string, int> batchDims_;
};

} // namespace stream
} // namespace data
} // namespace mlx
