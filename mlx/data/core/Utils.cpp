// Copyright © 2023 Apple Inc.

#include "mlx/data/core/Utils.h"
#include <iostream>

namespace {

using Array = mlx::data::Array;

template <class T>
void uniq_t(
    std::shared_ptr<Array> dst,
    std::shared_ptr<Array> dst_length,
    const std::shared_ptr<Array> src,
    const std::shared_ptr<Array> src_length,
    int dim,
    double pad) {
  int64_t stride = 1;
  for (int d = src->ndim() - 1; d > dim; d--) {
    stride *= src->shape(d);
  }

  int64_t niter = 1;
  for (int d = 0; d < src->ndim(); d++) {
    if (d != dim) {
      niter *= src->shape(d);
    }
  }
  auto dst_t = dst->data<T>();
  auto src_t = src->data<T>();
  auto src_length_t = src_length->data<int64_t>();
  auto dst_length_t = dst_length->data<int64_t>();
  auto max_sz = src->shape(dim);
  int64_t iterstride = (dim == src->ndim() - 1 ? src->shape(-1) : 1);
  for (int64_t iter = 0; iter < niter; iter++) {
    int64_t offset = iter * iterstride;
    int64_t sz = src_length_t[iter];
    int64_t idx = 0;
    if (sz > max_sz) {
      throw std::runtime_error("uniq: provided length exceeds input shape");
    }
    if (sz > 0) {
      T last = src_t[offset];
      dst_t[offset] = last;
      idx = 1;
      for (int64_t i = 1; i < sz; i++) {
        if (src_t[offset + i * stride] != last) {
          last = src_t[offset + i * stride];
          dst_t[offset + idx * stride] = last;
          idx++;
        }
      }
    }
    dst_length_t[iter] = idx;
    for (; idx < max_sz; idx++) {
      dst_t[offset + idx * stride] = static_cast<T>(pad);
    }
  }
}
template <class T>
void remove_t(
    std::shared_ptr<Array> dst,
    std::shared_ptr<Array> dst_length,
    const std::shared_ptr<Array> src,
    const std::shared_ptr<Array> src_length,
    int dim,
    double value,
    double pad) {
  int64_t stride = 1;
  for (int d = src->ndim() - 1; d > dim; d--) {
    stride *= src->shape(d);
  }

  int64_t niter = 1;
  for (int d = 0; d < src->ndim(); d++) {
    if (d != dim) {
      niter *= src->shape(d);
    }
  }
  auto dst_t = dst->data<T>();
  auto src_t = src->data<T>();
  auto src_length_t = src_length->data<int64_t>();
  auto dst_length_t = dst_length->data<int64_t>();
  auto max_sz = src->shape(dim);
  int64_t iterstride = (dim == src->ndim() - 1 ? src->shape(-1) : 1);
  for (int64_t iter = 0; iter < niter; iter++) {
    int64_t offset = iter * iterstride;
    int64_t sz = src_length_t[iter];
    int64_t idx = 0;
    if (sz > max_sz) {
      throw std::runtime_error("remove: provided length exceeds input shape");
    }
    for (int64_t i = 0; i < sz; i++) {
      if (src_t[offset + i * stride] != static_cast<T>(value)) {
        dst_t[offset + idx * stride] = src_t[offset + i * stride];
        idx++;
      }
    }
    dst_length_t[iter] = idx;
    for (; idx < max_sz; idx++) {
      dst_t[offset + idx * stride] = static_cast<T>(pad);
    }
  }
}
} // namespace
namespace mlx {
namespace data {
namespace core {

std::pair<std::shared_ptr<Array>, std::shared_ptr<Array>> uniq(
    const std::shared_ptr<Array> src,
    const std::shared_ptr<Array> src_length,
    int dim,
    double pad) {
  dim = src->checkdim(dim);
  if (src_length->size() != src->size() / src->shape(dim)) {
    throw std::runtime_error("uniq: array and length array do not match");
  }
  if (src_length->type() != ArrayType::Int64) {
    throw std::runtime_error("uniq: expected int64 for length array");
  }
  auto dst = std::make_shared<Array>(src->type(), src->shape());
  auto dst_length =
      std::make_shared<Array>(ArrayType::Int64, src_length->shape());
  ARRAY_DISPATCH(src, uniq_t, dst, dst_length, src, src_length, dim, pad);
  return std::make_pair(dst, dst_length);
}

std::pair<std::shared_ptr<Array>, std::shared_ptr<Array>> remove(
    const std::shared_ptr<Array> src,
    const std::shared_ptr<Array> src_length,
    int dim,
    double value,
    double pad) {
  dim = src->checkdim(dim);
  if (src_length->size() != src->size() / src->shape(dim)) {
    throw std::runtime_error("remove: array and length array do not match");
  }
  if (src_length->type() != ArrayType::Int64) {
    throw std::runtime_error("remove: expected int64 for length array");
  }
  auto dst = std::make_shared<Array>(src->type(), src->shape());
  auto dst_length =
      std::make_shared<Array>(ArrayType::Int64, src_length->shape());
  ARRAY_DISPATCH(
      src, remove_t, dst, dst_length, src, src_length, dim, value, pad);
  return std::make_pair(dst, dst_length);
}

Sample merge_batch(
    const std::vector<Sample>& samples,
    const std::unordered_map<std::string, double>& pad_values,
    const std::unordered_map<std::string, int>& batch_dims) {
  std::vector<std::string> keys;
  std::vector<std::vector<std::shared_ptr<Array>>> kvalues;
  for (auto& sample : samples) {
    if (keys.empty()) {
      keys = sample::keys(sample);
      kvalues.resize(keys.size());
    }
    for (int k = 0; k < keys.size(); k++) {
      auto& key = keys[k];
      auto kvalue = sample.find(key);
      if (kvalue == sample.end()) {
        throw std::runtime_error(
            "mergeBatch: inconsistent sample keys in batch (unknown key: <" +
            key + ">)");
      }
      kvalues[k].push_back(kvalue->second);
    }
  }
  if (keys.empty()) {
    return Sample();
  }
  Sample sample_batch;
  for (int k = 0; k < keys.size(); k++) {
    auto& key = keys[k];
    double pad_value = 0.0;
    auto kpad_value = pad_values.find(key);
    if (kpad_value != pad_values.end()) {
      pad_value = kpad_value->second;
    }
    auto kbatch_dim = batch_dims.find(key);
    if (kbatch_dim == batch_dims.end()) {
      sample_batch[key] = array::batch(kvalues[k], pad_value);
    } else {
      sample_batch[key] =
          array::batch(kvalues[k], kbatch_dim->second, pad_value);
    }
  }

  return sample_batch;
}

} // namespace core
} // namespace data
} // namespace mlx
