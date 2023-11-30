// Copyright © 2023 Apple Inc.

#pragma once

#include <memory>
#include <string>
#include <unordered_map>

#include "mlx/data/op/Op.h"
#include "mlx/data/stream/Stream.h"

namespace mlx {
namespace data {
namespace stream {

class Transform : public Stream {
 public:
  Transform(
      const std::shared_ptr<Stream>& stream,
      const std::shared_ptr<op::Op>& op);
  Transform(
      const std::shared_ptr<Stream>& stream,
      const std::vector<std::shared_ptr<op::Op>>& ops);

  virtual Sample next() const override;
  virtual void reset() override;

 protected:
  std::shared_ptr<Stream> stream_;
  std::vector<std::shared_ptr<op::Op>> ops_;
};

} // namespace stream
} // namespace data
} // namespace mlx
