// Copyright © 2023 Apple Inc.

#pragma once

#include <filesystem>
#include <string>

#include "mlx/data/op/KeyTransform.h"

namespace mlx {
namespace data {
namespace op {

/// Operation that will load a file into memory -- similar to `OpReadFromTAR`
/// but loads directly from the filesystem.  This is useful for testing the
/// in-memory path.
class LoadFile : public KeyTransformOp {
 public:
  LoadFile(
      const std::string& ikey,
      const std::filesystem::path& prefix = "",
      const std::string& okey = "");

  virtual std::shared_ptr<Array> apply_key(
      const std::shared_ptr<const Array>& src) const override;

 private:
  std::filesystem::path prefix_;
};

} // namespace op
} // namespace data
} // namespace mlx
