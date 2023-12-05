// Copyright © 2023 Apple Inc.

#pragma once

#include <utility>
#include "mlx/data/buffer/Batch.h"

namespace mlx {
namespace data {
namespace buffer {

class DynamicBatch : public Batch {
 public:
  DynamicBatch(
      const std::shared_ptr<Buffer>& buffer,
      const std::string& key,
      int64_t max_data_size = 0, // batch everything if <= 0
      const std::unordered_map<std::string, double>& pad_values = {},
      const std::unordered_map<std::string, int>& batch_dims = {});

  DynamicBatch(
      const std::shared_ptr<Buffer>& buffer,
      const std::shared_ptr<Buffer>& ref_size_buffer,
      const std::string& key,
      int64_t max_data_size,
      const std::unordered_map<std::string, double>& pad_values = {},
      const std::unordered_map<std::string, int>& batch_dims = {});

 private:
  DynamicBatch(
      std::pair<std::shared_ptr<Buffer>, std::vector<int64_t>>
          buffer_with_sizes,
      const std::unordered_map<std::string, double>& pad_values,
      const std::unordered_map<std::string, int>& batch_dims);

  // returns sorted buffer with number of samples for each batch
  static std::pair<std::shared_ptr<Buffer>, std::vector<int64_t>>
  dynamic_batch_(
      const std::shared_ptr<Buffer>& buffer,
      const std::shared_ptr<Buffer>& ref_sizebuffer,
      const std::string& key,
      int64_t max_data_size,
      const std::unordered_map<std::string, int>& batch_dims);
};

} // namespace buffer
} // namespace data
} // namespace mlx
