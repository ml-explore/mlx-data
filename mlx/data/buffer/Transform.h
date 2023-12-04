// Copyright © 2023 Apple Inc.

#pragma once

#include <memory>
#include <string>
#include <unordered_map>

#include "mlx/data/buffer/Buffer.h"
#include "mlx/data/op/Op.h"

namespace mlx {
namespace data {
namespace buffer {

class Transform : public Buffer {
 public:
  Transform(
      const std::shared_ptr<Buffer>& od,
      const std::shared_ptr<op::Op>& op);
  Transform(
      const std::shared_ptr<Buffer>& od,
      const std::vector<std::shared_ptr<op::Op>>& ops);

  virtual Sample get(int64_t idx) const override;

  virtual int64_t size() const override;

 protected:
  std::shared_ptr<Buffer> od_;
  std::vector<std::shared_ptr<op::Op>> ops_;
};

} // namespace buffer
} // namespace data
} // namespace mlx
