// Copyright © 2023 Apple Inc.

#pragma once

#include "mlx/data/Dataset.h"
#include "mlx/data/buffer/Buffer.h"

namespace mlx {
namespace data {

// Forward declaration of Stream so we can define toStream().
class Stream;

class Buffer : public Dataset<Buffer, buffer::Buffer> {
 public:
  Buffer(const std::shared_ptr<buffer::Buffer>& self);

  Sample get(int64_t idx) const;
  int64_t size() const;

  Buffer batch(
      int64_t batch_size,
      const std::unordered_map<std::string, double>& pad_values = {},
      const std::unordered_map<std::string, int>& batch_dims = {}) const;
  Buffer batch(
      const std::vector<int64_t>& batch_sizes,
      const std::unordered_map<std::string, double>& pad_values = {},
      const std::unordered_map<std::string, int>& batch_dims = {}) const;

  Buffer dynamic_batch(
      const std::string& key,
      int64_t max_data_size = 0, // batch everything if <= 0
      const std::unordered_map<std::string, double>& pad_values = {},
      const std::unordered_map<std::string, int>& batch_dims = {}) const;

  Buffer dynamic_batch(
      const Buffer& size_buffer,
      const std::string& key,
      int64_t max_data_size,
      const std::unordered_map<std::string, double>& pad_values = {},
      const std::unordered_map<std::string, int>& batch_dims = {}) const;

  Buffer partition(int64_t num_partitions, int64_t partition) const;
  Buffer partition_if(bool cond, int64_t num_partitions, int64_t partition)
      const;

  Buffer perm(const std::vector<int64_t>& perm);

  Buffer shuffle();
  Buffer shuffle_if(bool cond);

  Stream to_stream();

  friend class Stream;
};

Buffer buffer_from_vector(const std::vector<Sample>& data);
Buffer buffer_from_vector(std::vector<Sample>&& data);
Buffer files_from_tar(
    const std::string& tarfile,
    bool nested = false,
    int num_threads = 1);

} // namespace data
} // namespace mlx
