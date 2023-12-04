// Copyright © 2023 Apple Inc.

#pragma once

#include "mlx/data/buffer/Perm.h"

namespace mlx {
namespace data {
namespace buffer {

class Shuffle : public Perm {
 public:
  Shuffle(const std::shared_ptr<Buffer>& buffer);

 private:
  std::vector<int64_t> rand_perm_(int64_t size);
};

} // namespace buffer
} // namespace data
} // namespace mlx
