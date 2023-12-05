// Copyright © 2023 Apple Inc.

#pragma once

#include <memory>
#include <shared_mutex>
#include <string>
#include <unordered_map>

#include "mlx/data/stream/Stream.h"

namespace mlx {
namespace data {
namespace stream {

class Repeat : public Stream {
 public:
  Repeat(const std::shared_ptr<Stream>& stream, int64_t num_time);

  virtual Sample next() const override;
  virtual void reset() override;

 protected:
  std::shared_ptr<Stream> stream_;
  int64_t numTime_;
  mutable std::shared_mutex stream_reset_mutex_;
  mutable int64_t numDone_;
};

} // namespace stream
} // namespace data
} // namespace mlx
