// Copyright © 2023 Apple Inc.

#pragma once

#include "mlx/data/op/KeyTransform.h"

namespace mlx {
namespace data {
namespace op {

class LoadImage : public KeyTransformOp {
 public:
  // note: info=true is meant to fast-retrieval of image size and thus will not
  // perform transformations
  LoadImage(
      const std::string& ikey,
      const std::string& prefix = "",
      bool info = false,
      const std::string& format = "RGB",
      bool from_memory = false,
      const std::string& okey = "");

  virtual std::shared_ptr<Array> apply_key(
      const std::shared_ptr<const Array>& src) const override;

 private:
  std::string prefix_;
  bool info_;
  std::string format_;
  bool from_memory_;
};

} // namespace op
} // namespace data
} // namespace mlx
