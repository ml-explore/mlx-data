// Copyright © 2023 Apple Inc.

#pragma once

#include <atomic>
#include <mutex>

#include "mlx/data/buffer/Buffer.h"
#include "mlx/data/core/ThreadPool.h"
#include "mlx/data/stream/Stream.h"

namespace mlx {
namespace data {
namespace stream {

class OrderedPrefetch : public Stream {
 public:
  OrderedPrefetch(
      const std::shared_ptr<buffer::Buffer>& stream,
      int prefetch_size,
      int num_thread);
  ~OrderedPrefetch();

  virtual Sample next() const override;
  virtual void reset() override;

 private:
  std::shared_ptr<buffer::Buffer> buffer_;
  std::shared_ptr<core::ThreadPool> pool_;
  int64_t prefetchSize_;
  mutable int64_t currentIdx_;
  mutable std::vector<std::future<Sample>> prefetchCache_;
  mutable std::mutex mutex_;
};

} // namespace stream
} // namespace data
} // namespace mlx
