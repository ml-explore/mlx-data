// Copyright © 2023 Apple Inc.

#pragma once

#include "mlx/data/op/Op.h"

namespace mlx {
namespace data {
namespace op {

class SaveImage : public Op {
 public:
  SaveImage(
      const std::string& image_key,
      const std::string& filename_key,
      const std::string& prefix = "",
      const std::string& filename_prefix = "");

  virtual Sample apply(const Sample& sample) const override;

 private:
  std::string imageKey_;
  std::string filenameKey_;
  std::string prefix_;
  std::string filenamePrefix_;
};

} // namespace op
} // namespace data
} // namespace mlx
