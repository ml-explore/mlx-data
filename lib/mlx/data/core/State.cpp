#include "mlx/data/core/State.h"

namespace mlx {
namespace data {
namespace core {

static State global_state;

void set_state(int64_t seed) {
  global_state.randomGenerator = std::mt19937(seed);
  global_state.version++;
}

std::shared_ptr<State> get_state() {
  static thread_local std::shared_ptr<State> state;
  if (!state || (state->version != global_state.version)) {
    state = std::make_shared<State>(global_state);
  }
  return state;
};

} // namespace core
} // namespace data
} // namespace mlx
