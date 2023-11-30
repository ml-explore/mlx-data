// Copyright © 2023 Apple Inc.

#pragma once

#include "mlx/data/Sample.h"

namespace mlx {
namespace data {
namespace buffer {

class Buffer {
 public:
  Buffer(){};

  // User-specific
  virtual Sample get(int64_t idx) const;
  virtual int64_t size() const;

  virtual ~Buffer();
};

} // namespace buffer
} // namespace data
} // namespace mlx
