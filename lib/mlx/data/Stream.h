// Copyright © 2023 Apple Inc.

#pragma once

#include "mlx/data/Dataset.h"
#include "mlx/data/stream/Stream.h"

namespace mlx {
namespace data {

// Forward declaration of Buffer so we can define toBuffer() and buffered().
class Buffer;

class Stream : public Dataset<Stream, stream::Stream> {
 public:
  Stream(const std::shared_ptr<stream::Stream>& self);

  Sample next() const;
  void reset();

  Stream batch(
      int64_t batch_size,
      const std::unordered_map<std::string, double>& pad_values = {},
      const std::unordered_map<std::string, int>& batch_dims = {}) const;

  Stream buffered(
      int64_t buffer_size,
      std::function<Buffer(const Buffer)> on_refill,
      int num_thread) const;

  Stream csv_reader_from_key(
      const std::string& key,
      char sep = ',',
      char quote = '"',
      bool from_memory = false,
      const std::filesystem::path& local_prefix = "",
      std::shared_ptr<core::FileFetcher> fetcher = nullptr) const;

  Stream line_reader_from_key(
      const std::string& key,
      const std::string& dst_key,
      bool from_memory = false,
      bool unzip = false,
      const std::filesystem::path& local_prefix = "",
      std::shared_ptr<core::FileFetcher> fetcher = nullptr) const;

  Stream dynamic_batch(
      int64_t buffer_size,
      const std::string& key,
      int64_t max_data_size = 0, // batch everything if <= 0
      const std::unordered_map<std::string, double>& pad_values = {},
      const std::unordered_map<std::string, int>& batch_dims = {},
      bool shuffle = false,
      int num_thread = 1) const;

  Stream partition(int64_t num_partitions, int64_t partition) const;
  Stream partition_if(bool cond, int64_t num_partitions, int64_t partition)
      const;

  Stream prefetch(int prefetch_size, int num_thread) const;
  Stream prefetch_if(bool cond, int prefetch_size, int num_thread) const;

  Stream repeat(int64_t num_time) const;

  Stream shuffle(int64_t buffer_size) const;
  Stream shuffle_if(bool cond, int64_t buffer_size) const;

  Stream sliding_window(
      const std::string& key,
      int64_t size,
      int64_t stride,
      int dim = -1) const;

  Buffer to_buffer();
};

Stream stream_csv_reader(
    const std::string& filename,
    char sep = ',',
    char quote = '"',
    const std::filesystem::path& local_prefix = "",
    std::shared_ptr<core::FileFetcher> fetcher = nullptr);

Stream stream_csv_reader(
    const std::shared_ptr<std::istream>& f,
    char sep = ',',
    char quote = '"',
    std::shared_ptr<core::FileFetcherHandle> file_handle = nullptr);

Stream stream_csv_reader_from_string(
    const std::string& contents,
    char sep = ',',
    char quote = '"');

Stream stream_line_reader(
    const std::string& filename,
    const std::string& key,
    bool unzip = false,
    const std::filesystem::path& local_prefix = "",
    std::shared_ptr<core::FileFetcher> fetcher = nullptr);

Stream stream_line_reader(
    const std::shared_ptr<std::istream>& f,
    const std::string& key,
    bool unzip = false,
    std::shared_ptr<core::FileFetcherHandle> file_handle = nullptr);

} // namespace data
} // namespace mlx
