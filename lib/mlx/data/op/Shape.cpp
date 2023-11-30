// Copyright © 2023 Apple Inc.

#include "mlx/data/op/Shape.h"

namespace mlx {
namespace data {
namespace op {
Shape::Shape(const std::string& ikey, int dim, const std::string& okey)
    : ikey_(ikey), dim_(dim), okey_(okey), fullShape_(false) {}
Shape::Shape(const std::string& ikey, const std::string& okey)
    : ikey_(ikey), okey_(okey), fullShape_(true) {}
Sample Shape::apply(const Sample& sample) const {
  auto input_array = sample::check_key(sample, ikey_, ArrayType::Any);
  std::shared_ptr<Array> output_array;
  if (fullShape_) {
    output_array = std::make_shared<Array>(input_array->shape());
  } else {
    auto dim = input_array->checkdim(dim_);
    output_array = std::make_shared<Array>(input_array->shape(dim));
  }
  auto res = sample;
  res[okey_] = output_array;
  return res;
}
} // namespace op
} // namespace data
} // namespace mlx
