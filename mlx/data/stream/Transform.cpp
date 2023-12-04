// Copyright © 2023 Apple Inc.

#include <stdexcept>

#include "mlx/data/stream/Transform.h"

namespace mlx {
namespace data {
namespace stream {

Transform::Transform(
    const std::shared_ptr<Stream>& stream,
    const std::shared_ptr<op::Op>& op)
    : stream_(stream), ops_({op}){};

Transform::Transform(
    const std::shared_ptr<Stream>& stream,
    const std::vector<std::shared_ptr<op::Op>>& ops)
    : stream_(stream), ops_(ops){};

Sample Transform::next() const {
  // Process the stream untill it is either exhausted or a sample is
  // generated. While doing so mark the skipped elements.
  Sample res;

  while (res.empty()) {
    auto sample = stream_->next();

    // We exhausted the stream so return an empty sample
    if (sample.empty()) {
      break;
    }

    // Got a sample let's transform it
    res = sample;
    for (auto& op : ops_) {
      res = op->apply(res);

      // Hmm we should skip it
      if (res.empty()) {
        break;
      }
    }
  }

  return res;
}

void Transform::reset() {
  stream_->reset();
}

} // namespace stream
} // namespace data
} // namespace mlx
