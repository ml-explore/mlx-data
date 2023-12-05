// Copyright © 2023 Apple Inc.

#pragma once

#include <functional>
#include <memory>
#include <shared_mutex>

#include "mlx/data/stream/Stream.h"

namespace mlx {
namespace data {
namespace stream {

class Compose : public Stream {
 public:
  Compose(
      std::shared_ptr<Stream>& stream,
      std::function<std::shared_ptr<Stream>(const Sample& sample)> op);

  virtual Sample next() const override;
  virtual void reset() override;

 protected:
  bool next_stream_() const;

  mutable std::shared_ptr<Stream> stream_;
  mutable std::shared_ptr<Stream> composedStream_;
  mutable std::shared_mutex mutex_;
  std::function<std::shared_ptr<Stream>(const Sample& sample)> op_;
};

} // namespace stream
} // namespace data
} // namespace mlx
