// Copyright © 2023 Apple Inc.

#pragma once

#include <functional>
#include <shared_mutex>

#include "mlx/data/buffer/Buffer.h"
#include "mlx/data/stream/Stream.h"

namespace mlx {
namespace data {
namespace stream {

class Buffered : public Stream {
 public:
  Buffered(
      const std::shared_ptr<Stream>& stream,
      int64_t buffer_size,
      std::function<std::shared_ptr<buffer::Buffer>(
          const std::shared_ptr<buffer::Buffer>&)> on_refill =
          on_refill_default,
      int num_thread = 1);

  virtual Sample next() const override;
  virtual void reset() override;

  static std::shared_ptr<buffer::Buffer> on_refill_default(
      const std::shared_ptr<buffer::Buffer>& buffer);

 private:
  std::future<std::shared_ptr<buffer::Buffer>> background_buffer_fetch_() const;

  std::shared_ptr<Stream> stream_; // underlying stream
  int64_t bufferSize_; // how many buffer items
  std::function<std::shared_ptr<buffer::Buffer>(
      const std::shared_ptr<buffer::Buffer>)>
      onRefill_; // operation to be performed on top of buffered items

  std::shared_ptr<core::ThreadPool> pool_;
  mutable int currentIndex_;
  mutable std::shared_ptr<buffer::Buffer> buffer_;
  mutable std::future<std::shared_ptr<buffer::Buffer>> nextBuffer_;
  mutable std::shared_mutex mutex_;
};

} // namespace stream
} // namespace data
} // namespace mlx
