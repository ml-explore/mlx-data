// Copyright © 2023 Apple Inc.

#pragma once

#include "mlx/data/stream/Buffered.h"

namespace mlx {
namespace data {
namespace stream {

class DynamicBatch : public Buffered {
 public:
  DynamicBatch(
      std::shared_ptr<Stream> stream,
      int64_t buffer_size,
      const std::string& key,
      int64_t max_data_size = 0, // batch everything if <= 0
      const std::unordered_map<std::string, double>& pad_values = {},
      const std::unordered_map<std::string, int>& batch_dims = {},
      bool shuffle = false,
      int num_thread = 1);

 protected:
  std::shared_ptr<buffer::Buffer> on_refill(
      const std::shared_ptr<buffer::Buffer>&) const override;

 private:
  int buffer_size_;
  std::string key_;
  int64_t max_data_size_;
  std::unordered_map<std::string, double> pad_values_;
  std::unordered_map<std::string, int> batch_dims_;
  bool shuffle_;
};

} // namespace stream
} // namespace data
} // namespace mlx
