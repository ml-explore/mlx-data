// Copyright © 2025 Apple Inc.

#pragma once

#include "mlx/data/op/KeyTransform.h"

namespace mlx {
namespace data {
namespace op {

class Slice : public KeyTransformOp {
 public:
  Slice(
      const std::string& ikey,
      int dim,
      int64_t start,
      int64_t end,
      const std::string& okey = "");
  Slice(
      const std::string& ikey,
      std::vector<int> dims,
      std::vector<int64_t> starts,
      std::vector<int64_t> ends,
      const std::string& okey = "");

  virtual std::shared_ptr<Array> apply_key(
      const std::shared_ptr<const Array>& src) const override;

 private:
  std::vector<int> dims_;
  std::vector<int64_t> starts_;
  std::vector<int64_t> ends_;
};

class RandomSlice : public KeyTransformOp {
 public:
  RandomSlice(
      const std::string& ikey,
      int dim,
      int64_t size,
      const std::string& okey = "");
  RandomSlice(
      const std::string& ikey,
      std::vector<int> dims,
      std::vector<int64_t> sizes,
      const std::string& okey = "");

  virtual std::shared_ptr<Array> apply_key(
      const std::shared_ptr<const Array>& src) const override;

 private:
  std::vector<int> dims_;
  std::vector<int64_t> sizes_;
};

} // namespace op
} // namespace data
} // namespace mlx
