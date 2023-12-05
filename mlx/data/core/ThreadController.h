// Copyright © 2023 Apple Inc.

#include <memory>
#include <vector>

namespace mlx {
namespace data {
namespace core {

typedef std::vector<int> ThreadControllerState;

struct ThreadControllerSym;

class ThreadController {
 public:
  ThreadController();

  ThreadControllerState limit();
  void restore(const ThreadControllerState& state);

 private:
  std::vector<std::shared_ptr<ThreadControllerSym>> symbols_;
};

} // namespace core
} // namespace data
} // namespace mlx
