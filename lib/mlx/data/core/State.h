// Copyright © 2023 Apple Inc.

#pragma once

#include <memory>
#include <random>

#include "mlx/data/core/ThreadPool.h"

namespace mlx {
namespace data {
namespace core {

struct State {
  std::mt19937 randomGenerator;
  int64_t version;
};

// thread-local state
std::shared_ptr<State> get_state();

// should be called in main thread only
void set_state(int64_t seed);

} // namespace core
} // namespace data
} // namespace mlx
