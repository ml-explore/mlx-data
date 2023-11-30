// Copyright © 2023 Apple Inc.

#pragma once

#include "mlx/data/buffer/FromVector.h"
#include "mlx/data/stream/Stream.h"

namespace mlx {
namespace data {
namespace buffer {

class FromStream : public FromVector {
 public:
  FromStream(const std::shared_ptr<stream::Stream>& stream, int64_t size = -1);

 private:
  static std::vector<Sample> bufferize_(
      std::shared_ptr<stream::Stream> stream,
      int64_t size);
};

} // namespace buffer
} // namespace data
} // namespace mlx
