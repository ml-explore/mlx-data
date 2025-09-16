// Copyright © 2023 Apple Inc.

#pragma once

#include "mlx/data/buffer/Buffer.h"

namespace mlx {
namespace data {
namespace buffer {

class Append : public Buffer {
 public:
  Append(
      const std::shared_ptr<Buffer>& buffer1,
      const std::shared_ptr<Buffer>& buffer2);

  Sample get(int64_t idx) const override;
  virtual int64_t size() const override;

 private:
  std::shared_ptr<Buffer> buffer1_;
  std::shared_ptr<Buffer> buffer2_;
};

} // namespace buffer
} // namespace data
} // namespace mlx
