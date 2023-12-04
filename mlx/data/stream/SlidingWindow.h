// Copyright © 2023 Apple Inc.

#pragma once

#include <mutex>
#include <queue>

#include "mlx/data/stream/Stream.h"

namespace mlx {
namespace data {
namespace stream {

class SlidingWindow : public Stream {
 public:
  SlidingWindow(
      const std::shared_ptr<Stream>& stream,
      const std::string& key,
      int64_t size,
      int64_t stride,
      int dim = -1);

  virtual Sample next() const override;
  virtual void reset() override;

 private:
  mutable std::mutex mutex_;
  mutable std::queue<Sample> buffer_;
  std::shared_ptr<Stream> stream_;
  std::string key_;
  int64_t size_;
  int64_t stride_;
  int dim_;
};

} // namespace stream
} // namespace data
} // namespace mlx
