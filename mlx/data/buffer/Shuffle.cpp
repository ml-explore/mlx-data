// Copyright © 2023 Apple Inc.

#include "mlx/data/buffer/Shuffle.h"
#include "mlx/data/core/State.h"

#include <algorithm>
#include <numeric> // iota

namespace mlx {
namespace data {
namespace buffer {

Shuffle::Shuffle(const std::shared_ptr<Buffer>& buffer)
    : Perm(buffer, rand_perm_(buffer->size())) {}

std::vector<int64_t> Shuffle::rand_perm_(int64_t size) {
  auto state = core::get_state();
  std::vector<int64_t> perm(size);
  std::iota(perm.begin(), perm.end(), 0);
  std::shuffle(perm.begin(), perm.end(), state->randomGenerator);
  return perm;
}

} // namespace buffer
} // namespace data
} // namespace mlx
