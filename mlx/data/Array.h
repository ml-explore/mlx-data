// Copyright © 2023 Apple Inc.

#pragma once

#include <cstring>
#include <limits>
#include <memory>
#include <stdexcept>
#include <string>
#include <vector>

namespace mlx {
namespace data {

enum ArrayType { Any, UInt8, Int8, Int32, Int64, Float, Double };

template <class>
inline constexpr bool dependent_false_v = false;

class Array {
 public:
  Array();
  Array(const std::shared_ptr<const Array>& src);
  Array(ArrayType type);
  Array(const std::string& str);
  Array(ArrayType type, const std::vector<int64_t>& shape);
  Array(
      ArrayType type,
      const std::vector<int64_t>& shape,
      std::shared_ptr<void> data);

  template <class... Args>
  Array(ArrayType type, Args... dim)
      : Array(type, std::vector<int64_t>({dim...})){};

  template <class T>
  Array(const std::vector<T>& vec) {
    init_(to_array_type<T>(), {static_cast<int64_t>(vec.size())});
    memcpy(data(), vec.data(), vec.size() * itemsize_);
  };

  template <class T>
  Array(T scalar) {
    init_(to_array_type<T>(), {});
    *reinterpret_cast<T*>(data()) = scalar;
  };

  template <class T>
  static ArrayType to_array_type() {
    if constexpr (std::is_same_v<T, char>) {
      return ArrayType::Int8;
    } else if constexpr (std::is_same_v<T, unsigned char>) {
      return ArrayType::UInt8;
    } else if constexpr (std::is_same_v<T, int32_t>) {
      return ArrayType::Int32;
    } else if constexpr (std::is_same_v<T, int64_t>) {
      return ArrayType::Int64;
    } else if constexpr (std::is_same_v<T, float>) {
      return ArrayType::Float;
    } else if constexpr (std::is_same_v<T, double>) {
      return ArrayType::Double;
    } else {
      static_assert(dependent_false_v<T>, "unsupported type");
    }
  };

  const std::vector<int64_t>& shape() const;
  int64_t shape(int d) const;
  int ndim() const;
  int64_t itemsize() const;

  template <class T>
  T* data() const {
    if (type_ != to_array_type<T>()) {
      throw std::runtime_error("Array: incompatible array type");
    }
    return reinterpret_cast<T*>(data_.get());
  };

  template <class T>
  T scalar() const {
    if (size() != 1) {
      throw std::runtime_error("Array: expected a scalar array");
    }
    if (type_ != to_array_type<T>()) {
      throw std::runtime_error("Array: incompatible array type");
    }
    return *reinterpret_cast<T*>(data_.get());
  };

  void* data() const;
  int64_t size() const;

  ArrayType type() const;
  void fill(double value);
  void squeeze(const std::vector<int>& dims);
  void reshape(const std::vector<int64_t>& shape);
  int checkdim(int dim) const;

  ~Array();

 private:
  void init_(
      ArrayType type,
      const std::vector<int64_t>& shape,
      std::shared_ptr<void> data = nullptr);
  ArrayType type_;
  std::vector<int64_t> shape_;
  std::shared_ptr<void> data_ = nullptr;
  int64_t itemsize_ = 0;
};

#define ARRAY_DISPATCH(arr, functpl, ...)                                    \
  {                                                                          \
    switch ((arr)->type()) {                                                 \
      case mlx::data::ArrayType::Int32:                                      \
        functpl<int32_t>(__VA_ARGS__);                                       \
        break;                                                               \
      case mlx::data::ArrayType::Int64:                                      \
        functpl<int64_t>(__VA_ARGS__);                                       \
        break;                                                               \
      case mlx::data::ArrayType::Float:                                      \
        functpl<float>(__VA_ARGS__);                                         \
        break;                                                               \
      case mlx::data::ArrayType::Double:                                     \
        functpl<double>(__VA_ARGS__);                                        \
        break;                                                               \
      case mlx::data::ArrayType::Int8:                                       \
        functpl<char>(__VA_ARGS__);                                          \
        break;                                                               \
      case mlx::data::ArrayType::UInt8:                                      \
        functpl<unsigned char>(__VA_ARGS__);                                 \
        break;                                                               \
      default:                                                               \
        throw std::runtime_error("Array: internal error: unsupported type"); \
    }                                                                        \
  }

namespace array {

std::shared_ptr<Array> clone(const std::shared_ptr<const Array>& arr);

std::shared_ptr<Array> reshape(
    const std::shared_ptr<const Array>& arr,
    const std::vector<int64_t>& shape);

std::shared_ptr<Array> squeeze(
    const std::shared_ptr<const Array>& arr,
    const std::vector<int>& dims);

std::shared_ptr<Array> pad(
    const std::shared_ptr<const Array>& arr,
    int dim,
    int64_t lpad,
    int64_t rpad,
    double value);

std::shared_ptr<Array> slice(
    const std::shared_ptr<const Array>& arr,
    int index0);

void copy(
    const std::shared_ptr<Array>& dst,
    const std::shared_ptr<const Array>& src);

std::shared_ptr<Array> batch(
    const std::vector<std::shared_ptr<Array>>& arrs,
    double pad_value);

std::shared_ptr<Array> batch(
    const std::vector<std::shared_ptr<Array>>& arrs,
    int dim,
    double pad_value);

std::shared_ptr<Array> sub(
    const std::shared_ptr<const Array>& arr,
    const std::vector<int64_t> offset,
    const std::vector<int64_t> shape);

template <class T, class F>
void apply_visitor(void* dst, void* src, int64_t size, const F func) {
  auto src_t = static_cast<T*>(src);
  auto dst_t = static_cast<T*>(dst);
  for (int64_t i = 0; i < size; i++) {
    dst_t[i] = func(src_t[i]);
  }
}

template <class F>
std::shared_ptr<Array> apply(
    const std::shared_ptr<const Array>& src,
    const F func) {
  auto dst = std::make_shared<Array>(src->shape(), src->type());
  ARRAY_DISPATCH(
      src, apply_visitor, dst->data(), src->data(), src->size(), func);
  return dst;
};

} // namespace array
} // namespace data
} // namespace mlx
