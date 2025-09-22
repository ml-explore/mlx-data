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
      int num_thread = 1);

  virtual Sample next() const override;
  virtual void reset() override;

  virtual ~Buffered();

 protected:
  virtual std::shared_ptr<buffer::Buffer> on_refill(
      const std::shared_ptr<buffer::Buffer>& buffer) const;

  void finish_background_tasks();

 private:
  std::future<std::shared_ptr<buffer::Buffer>> background_buffer_fetch_() const;

  std::shared_ptr<Stream> stream_; // underlying stream
  int64_t buffer_size_; // how many buffer items

  std::shared_ptr<core::ThreadPool> pool_;
  bool pool_is_alive_ = true;
  mutable int current_index_;
  mutable std::shared_ptr<buffer::Buffer> buffer_;
  mutable std::future<std::shared_ptr<buffer::Buffer>> next_buffer_;
  mutable std::shared_mutex mutex_;
  mutable std::shared_mutex pool_mutex_;
};

class CallbackBuffered : public Buffered {
 public:
  CallbackBuffered(
      const std::shared_ptr<Stream>& stream,
      int64_t buffer_size,
      std::function<std::shared_ptr<buffer::Buffer>(
          const std::shared_ptr<buffer::Buffer>&)> on_refill,
      int num_thread = 1);

  virtual std::shared_ptr<buffer::Buffer> on_refill(
      const std::shared_ptr<buffer::Buffer>& buffer) const override;

 private:
  std::function<std::shared_ptr<buffer::Buffer>(
      const std::shared_ptr<buffer::Buffer>&)>
      on_refill_;
};

} // namespace stream
} // namespace data
} // namespace mlx
