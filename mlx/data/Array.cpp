// Copyright © 2023 Apple Inc.

#include <cstring>
#include <memory>

#include "mlx/data/Array.h"
#include "mlx/data/core/BatchShape.h"

namespace mlx {
namespace data {

Array::Array() {
  init_(ArrayType::Any, std::vector<int64_t>(0));
}

Array::Array(const std::shared_ptr<const Array>& src)
    : type_(src->type_),
      shape_(src->shape_),
      data_(src->data_),
      itemsize_(src->itemsize_) {}

Array::Array(ArrayType type) {
  init_(type, std::vector<int64_t>());
}

Array::Array(ArrayType type, const std::vector<int64_t>& shape) {
  init_(type, shape);
}

Array::Array(
    ArrayType type,
    const std::vector<int64_t>& shape,
    std::shared_ptr<void> data) {
  init_(type, shape, data);
}

void* Array::data() const {
  return data_.get();
}

ArrayType Array::type() const {
  return type_;
}

const std::vector<int64_t>& Array::shape() const {
  return shape_;
}

int Array::ndim() const {
  return shape_.size();
}

int64_t Array::shape(int d) const {
  return shape_[checkdim(d)];
}

int64_t Array::size() const {
  int64_t size = 1;
  for (auto dim : shape_) {
    size *= dim;
  }
  return size;
}

int64_t Array::itemsize() const {
  return itemsize_;
}

void Array::init_(
    ArrayType type,
    const std::vector<int64_t>& shape,
    std::shared_ptr<void> data) {
  shape_ = shape;
  type_ = type;
  switch (type_) {
    case UInt8:
      itemsize_ = sizeof(unsigned char);
      break;
    case Int32:
      itemsize_ = sizeof(int32_t);
      break;
    case Int64:
      itemsize_ = sizeof(int64_t);
      break;
    case Float:
      itemsize_ = sizeof(float);
      break;
    case Double:
      itemsize_ = sizeof(double);
      break;
    case Int8:
      itemsize_ = sizeof(char);
      break;
    case Any:
      itemsize_ = 0;
      if (size() != 0) {
        throw std::runtime_error(
            "Array: cannot create a tensor of undetermined type");
      }
      break;
    default:
      throw std::runtime_error("Array: internal error: unknown type");
  }
  if (data) {
    data_ = data;
  } else {
    if (size() > 0) {
      data_ = std::shared_ptr<void>(std::malloc(size() * itemsize_), std::free);
    }
  }
}

Array::Array(const std::string& str) {
  init_(ArrayType::Int8, {static_cast<int64_t>(str.size())});
  std::memcpy(data_.get(), str.data(), str.size());
}

Array::Array(std::string_view str) {
  init_(ArrayType::Int8, {static_cast<int64_t>(str.size())});
  std::memcpy(data_.get(), str.data(), str.size());
}

template <class T>
void array_fill(void* data, int64_t size, double value) {
  auto data_t = reinterpret_cast<T*>(data);
  auto value_t = static_cast<T>(value);
  for (int64_t i = 0; i < size; i++) {
    data_t[i] = value_t;
  }
}

void Array::fill(double value) {
  ARRAY_DISPATCH(this, array_fill, data_.get(), size(), value);
}

void Array::squeeze(const std::vector<int>& dims) {
  std::vector<int64_t> mask(ndim(), true);
  if (dims.empty()) {
    for (int i = 0; i < ndim(); i++) {
      if (shape_[i] == 1) {
        mask[i] = false;
      }
    }
  } else {
    for (auto dim : dims) {
      dim = checkdim(dim);
      if (shape_[dim] == 1) {
        mask[dim] = false;
      } else {
        throw std::runtime_error(
            "Array: cannot squeeze a non-singleton dimension");
      }
    }
  }
  int newndim = 0;
  for (int i = 0; i < shape_.size(); i++) {
    if (mask[i]) {
      if (i != newndim) {
        shape_[newndim] = shape_[i];
      }
      newndim++;
    }
  }
  shape_.resize(newndim);
}

void Array::reshape(const std::vector<int64_t>& shape) {
  std::vector<int64_t> new_shape = shape;
  int64_t new_size = 1;
  int infer_dim = -1;
  for (int dim = 0; dim < new_shape.size(); dim++) {
    if (new_shape[dim] < 0) {
      if (infer_dim >= 0) {
        throw std::runtime_error("Array: can infer only one dimension");
      } else {
        infer_dim = dim;
      }
    } else {
      new_size *= new_shape[dim];
    }
  }
  int64_t old_size = size();
  if (infer_dim >= 0) {
    if (old_size % new_size == 0) {
      new_shape[infer_dim] = old_size / new_size;
      new_size *= new_shape[infer_dim];
    } else {
      throw std::runtime_error(
          "Array: cannot infer dimension: incompatible shape provided");
    }
  }
  if (old_size != new_size) {
    throw std::runtime_error("Array: incompatible shape provided");
  }
  shape_ = new_shape;
}

Array::~Array() {}

int Array::checkdim(int dim) const {
  if (dim < 0) {
    dim = ndim() + dim;
  }
  if (dim < ndim()) {
    return dim;
  } else {
    throw std::runtime_error("Array: out of bound dimension");
  }
}

namespace array {

std::shared_ptr<Array> clone(const std::shared_ptr<const Array>& arr) {
  auto dst = std::make_shared<Array>(arr->type(), arr->shape());
  std::memcpy(dst->data(), arr->data(), arr->size() * arr->itemsize());
  return dst;
}

std::shared_ptr<Array> reshape(
    const std::shared_ptr<const Array>& arr,
    const std::vector<int64_t>& shape) {
  auto dst = std::make_shared<Array>(arr);
  dst->reshape(shape);
  return dst;
}

std::shared_ptr<Array> squeeze(
    const std::shared_ptr<const Array>& arr,
    const std::vector<int>& dims) {
  auto dst = std::make_shared<Array>(arr);
  dst->squeeze(dims);
  return dst;
}

template <class T>
void array_pad(
    void* dst,
    void* src,
    int64_t lchunksize,
    int64_t chunksize,
    int64_t rchunksize,
    int64_t size,
    int64_t itemsize) {
  auto dst_t = reinterpret_cast<T*>(dst);
  auto src_t = reinterpret_cast<T*>(src);
  for (int64_t n = 0; n < size; n++) {
    dst_t += lchunksize;
    std::memcpy(dst_t, src_t, chunksize * itemsize);
    dst_t += chunksize + rchunksize;
    src_t += chunksize;
  }
}

std::shared_ptr<Array> pad(
    const std::shared_ptr<const Array>& arr,
    int dim,
    int64_t lpad,
    int64_t rpad,
    double value) {
  if (lpad == 0 && rpad == 0) {
    return std::make_shared<Array>(arr);
  }
  if (lpad < 0 || rpad < 0) {
    throw std::runtime_error("Array: pad must be positive");
  }
  auto ndim = arr->ndim();
  if (dim < 0 || dim >= ndim) {
    throw std::runtime_error("Array: dim out of range");
  }
  auto shape = arr->shape();
  int64_t lchunksize = lpad;
  int64_t chunksize = shape[dim];
  int64_t rchunksize = rpad;
  int64_t n = 1;
  for (int d = dim + 1; d < ndim; d++) {
    lchunksize *= shape[d];
    chunksize *= shape[d];
    rchunksize *= shape[d];
  }
  for (int d = 0; d < dim; d++) {
    n *= shape[d];
  }
  shape[dim] += (lpad + rpad);
  auto res = std::make_shared<Array>(arr->type(), shape);
  res->fill(value);

  ARRAY_DISPATCH(
      arr,
      array_pad,
      res->data(),
      arr->data(),
      lchunksize,
      chunksize,
      rchunksize,
      n,
      arr->itemsize());

  return res;
}

std::shared_ptr<Array> slice(
    const std::shared_ptr<const Array>& arr,
    int index0) {
  // build a new shape dropping the first dimension
  auto new_shape = std::vector<int64_t>();
  auto current_shape = arr->shape();

  for (int i = 1; i < current_shape.size(); i++) {
    new_shape.push_back(current_shape[i]);
  }

  int64_t slice_size = (new_shape.size() > 0);
  for (auto dim : new_shape) {
    slice_size *= dim;
  }

  // return a slice of this at the given index
  auto item_size = arr->itemsize();
  auto new_pointer = ((char*)arr->data() + index0 * item_size * slice_size);
  std::shared_ptr<void> new_data((void*)new_pointer, [](void*) {});
  return std::make_shared<Array>(arr->type(), new_shape, new_data);
}

void copy(
    const std::shared_ptr<Array>& dst,
    const std::shared_ptr<const Array>& src) {
  if (dst->shape().size() != src->shape().size()) {
    throw std::runtime_error("Array::copy: src and dst sizes must match");
  }
  if (dst->itemsize() != src->itemsize()) {
    throw std::runtime_error("Array::copy: src and dst itemsize must match");
  }

  size_t size = src->size() * src->itemsize();
  memcpy(dst->data(), src->data(), size);
}

template <class T>
void array_copy_linear_to_strided(
    void* dst,
    int64_t offset,
    void* src,
    const std::vector<int64_t>& shape,
    const std::vector<int64_t>& stride) {
  auto dst_t = reinterpret_cast<T*>(dst);
  auto src_t = reinterpret_cast<T*>(src);

  // Deal with scalar arrays
  if (shape.size() == 0) {
    dst_t[offset] = src_t[0];
    return;
  }

  int ndim = shape.size();
  while (ndim > 1 && (shape[ndim - 1] == 1)) {
    ndim--;
  }
  // note: isize/istride considers the istride-contiguous portion
  // of the last dimensions (not only the last one)
  int64_t isize = shape[ndim - 1];
  int64_t istride = stride[ndim - 1];
  while ((ndim > 2) && (stride[ndim - 2] == isize * istride)) {
    isize *= shape[ndim - 2];
    ndim--;
  }
  int64_t osize = 1;
  for (int dim = 0; dim < ndim - 1; dim++) {
    osize *= shape[dim];
  }
  int64_t oprod = 1;
  for (int dim = 1; dim < ndim - 1; dim++) {
    oprod *= shape[dim];
  }
  if (istride == 1) {
    for (int64_t idx = 0; idx < osize; idx++) {
      int64_t roffset = offset;
      int64_t r = idx;
      int64_t rprod = oprod;
      for (int dim = 0; dim < ndim - 1; dim++) {
        int64_t rshape = r / rprod;
        r = r % rprod;
        rprod /= shape[dim + 1];
        roffset += stride[dim] * rshape;
      }
      std::memcpy(dst_t + roffset, src_t + idx * isize, isize * sizeof(T));
    }
  } else {
    for (int64_t idx = 0; idx < osize; idx++) {
      int64_t roffset = offset;
      int64_t r = idx;
      int64_t rprod = oprod;
      for (int dim = 0; dim < ndim - 1; dim++) {
        int64_t rshape = r / rprod;
        r = r % rprod;
        rprod /= shape[dim + 1];
        roffset += stride[dim] * rshape;
      }
      for (int64_t k = 0; k < isize; k++) {
        dst_t[roffset + k * istride] = src_t[idx * isize + k];
      }
    }
  }
}

template <class T>
void array_copy_strided_to_linear(
    void* dst,
    int64_t offset,
    void* src,
    const std::vector<int64_t>& shape,
    const std::vector<int64_t>& stride) {
  auto dst_t = reinterpret_cast<T*>(dst);
  auto src_t = reinterpret_cast<T*>(src);
  int ndim = shape.size();
  while (ndim > 1 && (shape[ndim - 1] == 1)) {
    ndim--;
  }
  // note: isize/istride considers the istride-contiguous portion
  // of the last dimensions (not only the last one)
  int64_t isize = shape[ndim - 1];
  int64_t istride = stride[ndim - 1];
  while ((ndim > 2) && (stride[ndim - 2] == isize * istride)) {
    isize *= shape[ndim - 2];
    ndim--;
  }
  int64_t osize = 1;
  for (int dim = 0; dim < ndim - 1; dim++) {
    osize *= shape[dim];
  }
  int64_t oprod = 1;
  for (int dim = 1; dim < ndim - 1; dim++) {
    oprod *= shape[dim];
  }
  if (istride == 1) {
    for (int64_t idx = 0; idx < osize; idx++) {
      int64_t roffset = offset;
      int64_t r = idx;
      int64_t rprod = oprod;
      for (int dim = 0; dim < ndim - 1; dim++) {
        int64_t rshape = r / rprod;
        r = r % rprod;
        rprod /= shape[dim + 1];
        roffset += stride[dim] * rshape;
      }
      std::memcpy(dst_t + idx * isize, src_t + roffset, isize * sizeof(T));
    }
  } else {
    for (int64_t idx = 0; idx < osize; idx++) {
      int64_t roffset = offset;
      int64_t r = idx;
      int64_t rprod = oprod;
      for (int dim = 0; dim < ndim - 1; dim++) {
        int64_t rshape = r / rprod;
        r = r % rprod;
        rprod /= shape[dim + 1];
        roffset += stride[dim] * rshape;
      }
      for (int64_t k = 0; k < isize; k++) {
        dst_t[idx * isize + k] = src_t[roffset + k * istride];
      }
    }
  }
}

std::shared_ptr<Array> batch(
    const std::vector<std::shared_ptr<Array>>& arrs,
    double pad_value) {
  core::BatchShape batch_shape;
  auto ndim = arrs.front()->ndim();
  auto type = arrs.front()->type();
  for (int i = 0; i < arrs.size(); i++) {
    if (arrs[i]->type() != type) {
      throw std::runtime_error(
          "Array: unexpected different types of arrays in batch");
    }
    batch_shape.add(arrs[i]->shape());
  }
  auto stride = std::vector<int64_t>(ndim);
  int64_t item_stride = 1;
  for (int dim = ndim - 1; dim >= 0; dim--) {
    stride[dim] = item_stride;
    item_stride *= batch_shape[dim + 1];
  }
  auto res = std::make_shared<Array>(type, batch_shape.shape());
  res->fill(pad_value);
  for (int i = 0; i < arrs.size(); i++) {
    auto arr = arrs[i];
    ARRAY_DISPATCH(
        arr,
        array_copy_linear_to_strided,
        res->data(),
        i * item_stride,
        arr->data(),
        arr->shape(),
        stride);
  }
  return res;
}

std::shared_ptr<Array> batch(
    const std::vector<std::shared_ptr<Array>>& arrs,
    int dim,
    double pad_value) {
  auto ndim = arrs.front()->ndim();
  auto type = arrs.front()->type();
  dim = arrs.front()->checkdim(dim);

  core::BatchShape batch_shape(dim);
  for (int i = 0; i < arrs.size(); i++) {
    if (arrs[i]->type() != type) {
      throw std::runtime_error(
          "Array: unexpected different types of arrays in batch");
    }
    batch_shape.add(arrs[i]->shape());
  }
  auto stride = std::vector<int64_t>(ndim);
  int64_t item_stride = 1;
  for (int d = ndim - 1; d >= 0; d--) {
    stride[d] = item_stride;
    item_stride *= batch_shape[d];
  }
  item_stride = 1;
  for (int d = ndim - 1; d > dim; d--) {
    item_stride *= batch_shape[d];
  }
  auto res = std::make_shared<Array>(type, batch_shape.shape());
  res->fill(pad_value);
  int64_t offset = 0;
  for (int i = 0; i < arrs.size(); i++) {
    auto arr = arrs[i];
    ARRAY_DISPATCH(
        arr,
        array_copy_linear_to_strided,
        res->data(),
        offset,
        arr->data(),
        arr->shape(),
        stride);
    offset += item_stride * arr->shape(dim);
  }
  return res;
}

std::shared_ptr<Array> sub(
    const std::shared_ptr<const Array>& arr,
    std::vector<int64_t> offset,
    std::vector<int64_t> shape) {
  if (arr->ndim() != offset.size()) {
    throw std::runtime_error("Array: sub: array and offset dim mismatch");
  }
  if (arr->ndim() != shape.size()) {
    throw std::runtime_error("Array: sub: array and shape dim mismatch");
  }

  int64_t offset_sum = 0;
  std::vector<int64_t> stride(arr->ndim(), 1);
  for (int64_t dim = arr->ndim() - 1; dim >= 0; dim--) {
    if (offset[dim] < 0 || offset[dim] >= arr->shape(dim)) {
      throw std::runtime_error("Array: sub: offset out of bound");
    }
    if (shape[dim] < 0) {
      shape[dim] = arr->shape(dim);
    }
    if (offset[dim] + shape[dim] > arr->shape(dim)) {
      throw std::runtime_error("Array: sub: shape out of bound");
    }
    offset_sum += offset[dim] * stride[dim];
    if (dim > 0) {
      stride[dim - 1] = stride[dim] * arr->shape(dim);
    }
  }

  auto res = std::make_shared<Array>(arr->type(), shape);
  ARRAY_DISPATCH(
      arr,
      array_copy_strided_to_linear,
      res->data(),
      offset_sum,
      arr->data(),
      shape,
      stride);
  return res;
}

} // namespace array
} // namespace data
} // namespace mlx
