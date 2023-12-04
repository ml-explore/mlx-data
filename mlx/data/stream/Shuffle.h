// Copyright © 2023 Apple Inc.

#pragma once

#include <mutex>

#include "mlx/data/stream/Stream.h"

namespace mlx {
namespace data {
namespace stream {

class Shuffle : public Stream {
 public:
  Shuffle(const std::shared_ptr<Stream>& stream, int buffer_size);

  virtual Sample next() const override;
  virtual void reset() override;

 private:
  std::shared_ptr<Stream> stream_;
  int buffer_size_;
  mutable std::vector<Sample> buffer_;
  mutable std::mutex mutex_;
};

} // namespace stream
} // namespace data
} // namespace mlx
