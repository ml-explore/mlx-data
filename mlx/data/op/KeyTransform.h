// Copyright © 2023 Apple Inc.

#pragma once

#include <functional>

#include "mlx/data/op/Op.h"

namespace mlx {
namespace data {
namespace op {

class KeyTransformOp : public Op {
 public:
  KeyTransformOp(const std::string& ikey, const std::string& okey = "");

  virtual Sample apply(const Sample& sample) const;
  virtual std::shared_ptr<Array> apply_key(
      const std::shared_ptr<const Array>& x) const = 0;

 protected:
  std::string ikey_;
  std::string okey_;
};

class KeyTransform : public KeyTransformOp {
 public:
  KeyTransform(
      const std::string& ikey,
      std::function<std::shared_ptr<Array>(const std::shared_ptr<const Array>&)>
          op,
      const std::string& okey = "");

  virtual std::shared_ptr<Array> apply_key(
      const std::shared_ptr<const Array>& x) const override;

 private:
  std::function<std::shared_ptr<Array>(const std::shared_ptr<const Array>&)>
      op_;
};

} // namespace op
} // namespace data
} // namespace mlx
