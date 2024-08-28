// Copyright © 2023 Apple Inc.

#include "mlx/data/buffer/DynamicBatch.h"
#include "mlx/data/Sample.h"
#include "mlx/data/buffer/Perm.h"
#include "mlx/data/core/BatchShape.h"

#include <algorithm>
#include <numeric>

namespace mlx {
namespace data {
namespace buffer {

DynamicBatch::DynamicBatch(
    const std::shared_ptr<Buffer>& buffer,
    const std::string& key,
    int64_t max_data_size,
    const std::unordered_map<std::string, double>& pad_values,
    const std::unordered_map<std::string, int>& batch_dims)
    : DynamicBatch(
          buffer,
          nullptr,
          key,
          max_data_size,
          pad_values,
          batch_dims) {};

DynamicBatch::DynamicBatch(
    const std::shared_ptr<Buffer>& buffer,
    const std::shared_ptr<Buffer>& ref_size_buffer,
    const std::string& key,
    int64_t max_data_size,
    const std::unordered_map<std::string, double>& pad_values,
    const std::unordered_map<std::string, int>& batch_dims)
    : DynamicBatch(
          dynamic_batch_(
              buffer,
              ref_size_buffer,
              key,
              max_data_size,
              batch_dims),
          pad_values,
          batch_dims) {};

DynamicBatch::DynamicBatch(
    std::pair<std::shared_ptr<Buffer>, std::vector<int64_t>> buffer_with_sizes,
    const std::unordered_map<std::string, double>& pad_values,
    const std::unordered_map<std::string, int>& batch_dims)
    : Batch(
          buffer_with_sizes.first,
          buffer_with_sizes.second,
          pad_values,
          batch_dims) {};

std::pair<std::shared_ptr<Buffer>, std::vector<int64_t>>
DynamicBatch::dynamic_batch_(
    const std::shared_ptr<Buffer>& buffer,
    const std::shared_ptr<Buffer>& ref_size_buffer,
    const std::string& key,
    int64_t max_data_size,
    const std::unordered_map<std::string, int>& batch_dims) {
  int64_t n = buffer->size();

  if (ref_size_buffer && (ref_size_buffer->size() != n)) {
    throw std::runtime_error(
        "DynamicBatch: buffer and reference size buffer do not match in size");
  }

  // get sample shapes
  std::vector<std::vector<int64_t>> sample_shapes(n);
  for (int64_t i = 0; i < n; i++) {
    if (ref_size_buffer) {
      auto sample = ref_size_buffer->get(i);
      auto array = mlx::data::sample::check_key(sample, key, ArrayType::Int64);
      sample_shapes[i].insert(
          sample_shapes[i].begin(),
          array->data<int64_t>(),
          array->data<int64_t>() + array->size());
    } else {
      auto sample = buffer->get(i);
      auto array = mlx::data::sample::check_key(sample, key, ArrayType::Any);
      sample_shapes[i] = array->shape();
    }
  }

  // get sample sizes
  std::vector<int64_t> sample_sizes(n, 0);
  for (int64_t i = 0; i < n; i++) {
    auto& shape = sample_shapes[i];
    if (shape.size() > 0) {
      int64_t size = shape[0];
      for (int i = 1; i < shape.size(); i++) {
        size *= shape[i];
      }
      sample_sizes[i] = size;
    }
  }

  // sort (if we do not batch everything)
  std::vector<int64_t> perm(sample_sizes.size());
  std::iota(perm.begin(), perm.end(), 0);
  if (max_data_size > 0) {
    std::stable_sort(
        perm.begin(), perm.end(), [&sample_sizes](size_t i1, size_t i2) {
          return sample_sizes[i1] < sample_sizes[i2];
        });
  }

  std::vector<int64_t> num_sample_per_batch;
  core::BatchShape batch_shape;
  auto kbatch_dim = batch_dims.find(key);
  if (kbatch_dim != batch_dims.end()) {
    batch_shape = core::BatchShape(kbatch_dim->second);
  }

  for (int64_t i = 0; i < n; i++) {
    batch_shape.add(sample_shapes[perm[i]]);
    if ((max_data_size > 0) && (batch_shape.size() > max_data_size)) {
      if (batch_shape.num_sample() == 1) {
        num_sample_per_batch.push_back(1);
      } else {
        num_sample_per_batch.push_back(batch_shape.num_sample() - 1);
        batch_shape.clear();
        batch_shape.add(sample_shapes[perm[i]]); // the one we just rejected
      }
    }
  }
  if (batch_shape.num_sample()) {
    num_sample_per_batch.push_back(batch_shape.num_sample());
  }

  auto buffer_perm = std::make_shared<Perm>(buffer, perm);
  return std::make_pair(buffer_perm, std::move(num_sample_per_batch));
}

} // namespace buffer
} // namespace data
} // namespace mlx
