// Copyright © 2023 Apple Inc.

#pragma once

#include "mlx/data/buffer/Buffer.h"

namespace mlx {
namespace data {
namespace buffer {

class FromVector : public Buffer {
 public:
  FromVector(const std::vector<Sample>& data);
  FromVector(std::vector<Sample>&& data);

  Sample get(int64_t idx) const override;
  virtual int64_t size() const override;

 private:
  void check_samples_() const;
  std::vector<Sample> buffer_;
};

} // namespace buffer
} // namespace data
} // namespace mlx
