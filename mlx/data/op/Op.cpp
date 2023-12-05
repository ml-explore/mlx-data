// Copyright © 2023 Apple Inc.

#include <stdexcept>

#include "mlx/data/op/Op.h"

namespace mlx {
namespace data {
namespace op {

Sample Op::apply(const Sample& sample) const {
  throw std::runtime_error("Op::apply() NYI");
}

Op::~Op() {}

} // namespace op
} // namespace data
} // namespace mlx
