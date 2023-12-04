// Copyright © 2023 Apple Inc.

#pragma once

#include <mutex>

#include "mlx/data/buffer/Buffer.h"
#include "mlx/data/stream/Stream.h"

namespace mlx {
namespace data {
namespace stream {

class FromBuffer : public Stream {
 public:
  FromBuffer(const std::shared_ptr<buffer::Buffer>& buffer);

  virtual Sample next() const override;
  virtual void reset() override;

 private:
  std::shared_ptr<buffer::Buffer> buffer_;
  mutable int64_t currentIdx_;
  mutable std::mutex mutex_;
};

} // namespace stream
} // namespace data
} // namespace mlx
