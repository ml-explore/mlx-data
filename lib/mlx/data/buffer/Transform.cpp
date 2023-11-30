// Copyright © 2023 Apple Inc.

#include <stdexcept>

#include "mlx/data/buffer/Transform.h"

namespace mlx {
namespace data {
namespace buffer {

Transform::Transform(
    const std::shared_ptr<Buffer>& od,
    const std::shared_ptr<op::Op>& op)
    : od_(od), ops_({op}){};

Transform::Transform(
    const std::shared_ptr<Buffer>& od,
    const std::vector<std::shared_ptr<op::Op>>& ops)
    : od_(od), ops_(ops){};

Sample Transform::get(const int64_t idx) const {
  auto t_sample = od_->get(idx);
  if (t_sample.empty()) {
    throw std::runtime_error("Transform: cannot return empty sample");
  }
  for (auto& op : ops_) {
    t_sample = op->apply(t_sample);
    if (t_sample.empty()) {
      throw std::runtime_error("Transform: cannot return empty sample");
    }
  }
  return t_sample;
}

int64_t Transform::size() const {
  return od_->size();
}

} // namespace buffer
} // namespace data
} // namespace mlx
