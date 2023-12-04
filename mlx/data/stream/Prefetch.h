// Copyright © 2023 Apple Inc.

#pragma once

#include <atomic>
#include <mutex>

#include "mlx/data/core/ThreadPool.h"
#include "mlx/data/stream/Stream.h"

namespace mlx {
namespace data {
namespace stream {

class Prefetch : public Stream {
 public:
  Prefetch(
      const std::shared_ptr<Stream>& stream,
      int prefetch_size,
      int num_thread);
  ~Prefetch();

  virtual Sample next() const override;
  virtual void reset() override;

 private:
  std::shared_ptr<Stream> stream_;
  std::shared_ptr<core::ThreadPool> pool_;
  int prefetchSize_;
  mutable std::queue<std::future<Sample>> prefetchCache_;
  mutable std::mutex mutex_;
};

} // namespace stream
} // namespace data
} // namespace mlx
