// Copyright © 2023 Apple Inc.

#pragma once

#include "mlx/data/op/KeyTransform.h"

namespace mlx {
namespace data {
namespace op {

class Pad : public KeyTransformOp {
 public:
  Pad(const std::string& ikey,
      int dim,
      int64_t lpad,
      int64_t rpad,
      double value,
      const std::string& okey = "");

  virtual std::shared_ptr<Array> apply_key(
      const std::shared_ptr<const Array>& src) const override;

 private:
  int dim_;
  int64_t lpad_;
  int64_t rpad_;
  double value_;
};

class PadToSize : public KeyTransformOp {
 public:
  PadToSize(
      const std::string& ikey,
      int dim,
      int64_t size,
      double value,
      const std::string& okey = "");
  PadToSize(
      const std::string& ikey,
      int dim,
      const std::vector<int64_t>& sizes,
      double value,
      const std::string& okey = "");

  virtual std::shared_ptr<Array> apply_key(
      const std::shared_ptr<const Array>& src) const override;

 private:
  int dim_;
  std::vector<int64_t> sizes_;
  double value_;
};

class PadToMultiple : public KeyTransformOp {
 public:
  PadToMultiple(
      const std::string& ikey,
      int dim,
      int64_t size,
      double value,
      const std::string& okey = "");

  virtual std::shared_ptr<Array> apply_key(
      const std::shared_ptr<const Array>& src) const override;

 private:
  int dim_;
  int64_t size_;
  double value_;
};

} // namespace op
} // namespace data
} // namespace mlx
