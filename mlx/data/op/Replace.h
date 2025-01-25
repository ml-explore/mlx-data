// Copyright © 2024 Apple Inc.

#pragma once

#include <vector>

#include "mlx/data/op/KeyTransform.h"

namespace mlx {
namespace data {
namespace op {

class Replace : public KeyTransformOp {
 public:
  Replace(
      const std::string& key,
      const std::string& old,
      const std::string& replacement,
      int count);

  virtual std::shared_ptr<Array> apply_key(
      const std::shared_ptr<const Array>& src) const override;

 private:
  std::string key_;
  std::shared_ptr<Array> old_;
  std::shared_ptr<Array> replacement_;
  int count_;
};

class ReplaceBytes : public KeyTransformOp {
 public:
  ReplaceBytes(
      const std::string& ikey,
      std::vector<std::string> byte_map,
      const std::string& okey = "");

  virtual std::shared_ptr<Array> apply_key(
      const std::shared_ptr<const Array>& src) const override;

 private:
  std::vector<std::string> byte_map_;
};

} // namespace op
} // namespace data
} // namespace mlx
