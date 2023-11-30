// Copyright © 2023 Apple Inc.

#pragma once

#include <future>
#include <vector>

#include "mlx/data/Sample.h"
#include "mlx/data/core/State.h"

namespace mlx {
namespace data {
namespace stream {

class Stream {
 public:
  Stream(){};

  // fetch next sample
  virtual Sample next() const;

  // reset the stream
  virtual void reset();

  virtual ~Stream();
};

} // namespace stream
} // namespace data
} // namespace mlx
