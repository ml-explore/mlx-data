#include <stdexcept>

#include "mlx/data/core/BatchShape.h"

namespace mlx {
namespace data {
namespace core {

BatchShape::BatchShape() : nodim_(true), num_sample_(0){};
BatchShape::BatchShape(int dim) : dim_(dim), nodim_(false), num_sample_(0){};

int64_t BatchShape::size() const {
  int64_t size = 1;
  for (auto dim : shape_) {
    size *= dim;
  }
  return size;
}

const std::vector<int64_t>& BatchShape::shape() const {
  return shape_;
}

void BatchShape::add(const std::vector<int64_t>& shape) {
  if (nodim_) {
    if (num_sample_ == 0) {
      shape_.resize(shape.size() + 1, 0);
      std::copy(shape.begin(), shape.end(), shape_.begin() + 1);
    } else {
      if ((shape.size() + 1) != shape_.size()) {
        throw std::runtime_error(
            "BatchShape: batched arrays expected to have consistent shapes");
      }
      for (int d = 0; d < shape.size(); d++) {
        shape_[d + 1] = std::max(shape_[d + 1], shape[d]);
      }
    }
    shape_[0] += 1;
  } else {
    int64_t dim = dim_;
    if (dim < 0) {
      dim = shape.size() + dim;
    }
    if (dim >= shape.size()) {
      throw std::runtime_error("BatchShape: dimension out of bound");
    }
    if (num_sample_ == 0) {
      shape_ = shape;
    } else {
      if (shape.size() != shape_.size()) {
        throw std::runtime_error(
            "BatchShape: batched arrays expected to have consistent shapes");
      }
      for (int d = 0; d < shape.size(); d++) {
        if (d == dim) {
          shape_[d] += shape[d];
        } else {
          shape_[d] = std::max(shape_[d], shape[d]);
        }
      }
    }
  }
  num_sample_++;
}

int64_t BatchShape::num_sample() const {
  return num_sample_;
}

void BatchShape::clear() {
  shape_.clear();
  num_sample_ = 0;
}

int64_t BatchShape::operator[](int dim) const {
  if (dim < 0) {
    dim = shape_.size() + dim;
  }
  if (dim >= shape_.size()) {
    throw std::runtime_error("BatchShape: dimension out of bound");
  }
  return shape_[dim];
}

} // namespace core
} // namespace data
} // namespace mlx
