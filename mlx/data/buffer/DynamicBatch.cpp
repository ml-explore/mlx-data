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
    int64_t min_data_size,
    int64_t max_data_size,
    const std::unordered_map<std::string, double>& pad_values,
    const std::unordered_map<std::string, int>& batch_dims,
    bool drop_outliers)
    : DynamicBatch(
          buffer,
          nullptr,
          key,
          min_data_size,
          max_data_size,
          pad_values,
          batch_dims,
          drop_outliers) {};

DynamicBatch::DynamicBatch(
    const std::shared_ptr<Buffer>& buffer,
    const std::shared_ptr<Buffer>& ref_size_buffer,
    const std::string& key,
    int64_t min_data_size,
    int64_t max_data_size,
    const std::unordered_map<std::string, double>& pad_values,
    const std::unordered_map<std::string, int>& batch_dims,
    bool drop_outliers)
    : DynamicBatch(
          dynamic_batch_(
              buffer,
              ref_size_buffer,
              key,
              min_data_size,
              max_data_size,
              batch_dims,
              drop_outliers),
          pad_values,
          batch_dims) {};

DynamicBatch::DynamicBatch(
    std::tuple<
        std::shared_ptr<Buffer>,
        std::vector<int64_t>,
        std::vector<int64_t>> buffer_with_sizes,
    const std::unordered_map<std::string, double>& pad_values,
    const std::unordered_map<std::string, int>& batch_dims)
    : Batch(
          std::get<0>(buffer_with_sizes),
          std::get<1>(buffer_with_sizes),
          pad_values,
          batch_dims),
      skipped_samples_(std::get<2>(buffer_with_sizes)) {};

std::tuple<std::shared_ptr<Buffer>, std::vector<int64_t>, std::vector<int64_t>>
DynamicBatch::dynamic_batch_(
    const std::shared_ptr<Buffer>& buffer,
    const std::shared_ptr<Buffer>& ref_size_buffer,
    const std::string& key,
    int64_t min_data_size,
    int64_t max_data_size,
    const std::unordered_map<std::string, int>& batch_dims,
    bool drop_outliers) {
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
  std::vector<int64_t> sorted_samples(sample_sizes.size());
  std::iota(sorted_samples.begin(), sorted_samples.end(), 0);
  if (max_data_size > 0) {
    std::stable_sort(
        sorted_samples.begin(),
        sorted_samples.end(),
        [&sample_sizes](size_t i1, size_t i2) {
          return sample_sizes[i1] < sample_sizes[i2];
        });
  }

  std::vector<int64_t> num_sample_per_batch;
  core::BatchShape batch_shape;
  auto kbatch_dim = batch_dims.find(key);
  if (kbatch_dim != batch_dims.end()) {
    batch_shape = core::BatchShape(kbatch_dim->second);
  }

  std::vector<int64_t> skipped_samples;
  std::vector<int64_t> accepted_samples;
  accepted_samples.reserve(sorted_samples.size());

  auto accept_samples =
      [&accepted_samples, &num_sample_per_batch, &batch_shape, &sorted_samples](
          int64_t last_idx, int64_t num_samples) {
        num_sample_per_batch.push_back(num_samples);
        int64_t first_idx = last_idx + 1 - num_samples;
        accepted_samples.insert(
            accepted_samples.end(),
            sorted_samples.begin() + first_idx,
            sorted_samples.begin() + last_idx + 1);
        batch_shape.clear();
      };

  auto skip_samples = [&skipped_samples, &sorted_samples, &batch_shape](
                          int64_t last_idx, int64_t num_samples) {
    int64_t first_idx = last_idx + 1 - num_samples;
    skipped_samples.insert(
        skipped_samples.end(),
        sorted_samples.begin() + first_idx,
        sorted_samples.begin() + last_idx + 1);
    batch_shape.clear();
  };

  for (int64_t i = 0; i < sorted_samples.size(); i++) {
    batch_shape.add(sample_shapes[sorted_samples[i]]);
    // do we match in size?
    if ((min_data_size > 0) && (batch_shape.size() >= min_data_size)) {
      if ((max_data_size > 0) && (batch_shape.size() <= max_data_size)) {
        accept_samples(i, batch_shape.num_sample());
        continue;
      }
    }
    // did we go too far?
    if ((max_data_size > 0) && (batch_shape.size() > max_data_size)) {
      // one sample is too long
      if (batch_shape.num_sample() == 1) {
        if (!drop_outliers) {
          // let's keep it anyways
          accept_samples(i, 1);
        } else {
          // ignore
          batch_shape.clear();
        }
        continue;
      }

      // we would have had accepted the batch if it was large enough
      // given we did not, it is now too large
      // we thus skip these samples for now
      if (min_data_size > 0) {
        skip_samples(i, batch_shape.num_sample());
      } else {
        accept_samples(i - 1, batch_shape.num_sample() - 1);
        batch_shape.add(
            sample_shapes[sorted_samples[i]]); // the one we just rejected
      }
    }
  }
  if (batch_shape.num_sample()) {
    if (((min_data_size <= 0) || (batch_shape.size() >= min_data_size)) &&
        ((max_data_size <= 0) || (batch_shape.size() <= max_data_size))) {
      accept_samples(sorted_samples.size() - 1, batch_shape.num_sample());
    } else {
      skip_samples(sorted_samples.size() - 1, batch_shape.num_sample());
    }
  }

  return std::make_tuple(
      std::make_shared<Perm>(buffer, accepted_samples),
      std::move(num_sample_per_batch),
      std::move(skipped_samples));
}

} // namespace buffer
} // namespace data
} // namespace mlx
