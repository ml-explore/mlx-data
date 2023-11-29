#include "mlx/data/Stream.h"
#include "mlx/data/Buffer.h"
#include "mlx/data/buffer/FromStream.h"
#include "mlx/data/stream/Batch.h"
#include "mlx/data/stream/Buffered.h"
#include "mlx/data/stream/CSVReader.h"
#include "mlx/data/stream/DynamicBatch.h"
#include "mlx/data/stream/LineReader.h"
#include "mlx/data/stream/Partition.h"
#include "mlx/data/stream/Prefetch.h"
#include "mlx/data/stream/Repeat.h"
#include "mlx/data/stream/Shuffle.h"
#include "mlx/data/stream/SlidingWindow.h"

namespace mlx {
namespace data {

Stream::Stream(const std::shared_ptr<stream::Stream>& self)
    : Dataset<Stream, stream::Stream>(self){};

Sample Stream::next() const {
  return self_->next();
}

void Stream::reset() {
  self_->reset();
}

Stream Stream::batch(
    int64_t batch_size,
    const std::unordered_map<std::string, double>& pad_values,
    const std::unordered_map<std::string, int>& batch_dims) const {
  return Stream(std::make_shared<stream::Batch>(
      self_, batch_size, pad_values, batch_dims));
}

Stream Stream::buffered(
    int64_t buffer_size,
    std::function<Buffer(const Buffer)> on_refill,
    int num_thread) const {
  auto on_refill_stream =
      [on_refill](const std::shared_ptr<buffer::Buffer>& buf) {
        return on_refill(Buffer(buf)).self_;
      };
  return Stream(std::make_shared<stream::Buffered>(
      self_, buffer_size, on_refill_stream, num_thread));
};

Stream Stream::csv_reader_from_key(
    const std::string& key,
    char sep,
    char quote,
    bool from_memory,
    const std::filesystem::path& local_prefix,
    std::shared_ptr<core::FileFetcher> fetcher) const {
  return Stream(std::make_shared<stream::CSVReaderFromKey>(
      self_, key, sep, quote, from_memory, local_prefix, fetcher));
}

Stream Stream::line_reader_from_key(
    const std::string& key,
    const std::string& dst_key,
    bool from_memory,
    bool unzip,
    const std::filesystem::path& local_prefix,
    std::shared_ptr<core::FileFetcher> fetcher) const {
  return Stream(std::make_shared<stream::LineReaderFromKey>(
      self_, key, dst_key, from_memory, unzip, local_prefix, fetcher));
}

Stream Stream::dynamic_batch(
    int64_t buffer_size,
    const std::string& key,
    int64_t max_data_size,
    const std::unordered_map<std::string, double>& pad_values,
    const std::unordered_map<std::string, int>& batch_dims,
    bool shuffle,
    int num_thread) const {
  return Stream(std::make_shared<stream::DynamicBatch>(
      self_,
      buffer_size,
      key,
      max_data_size,
      pad_values,
      batch_dims,
      shuffle,
      num_thread));
}

Stream Stream::partition(int64_t num_partitions, int64_t partition) const {
  return Stream(
      std::make_shared<stream::Partition>(self_, num_partitions, partition));
}
Stream Stream::partition_if(
    bool cond,
    int64_t num_partitions,
    int64_t partition) const {
  if (cond) {
    return this->partition(num_partitions, partition);
  } else {
    return Stream(self_);
  }
}

Stream Stream::prefetch(int prefetch_size, int num_thread) const {
  return Stream(
      std::make_shared<stream::Prefetch>(self_, prefetch_size, num_thread));
}
Stream Stream::prefetch_if(bool cond, int prefetch_size, int num_thread) const {
  if (cond) {
    return prefetch(prefetch_size, num_thread);
  } else {
    return Stream(self_);
  }
}

Stream Stream::repeat(int64_t num_time) const {
  return Stream(std::make_shared<stream::Repeat>(self_, num_time));
}

Stream Stream::shuffle(int64_t buffer_size) const {
  return Stream(std::make_shared<stream::Shuffle>(self_, buffer_size));
}
Stream Stream::shuffle_if(bool cond, int64_t buffer_size) const {
  if (cond) {
    return shuffle(buffer_size);
  } else {
    return Stream(self_);
  }
}

Stream Stream::sliding_window(
    const std::string& key,
    int64_t size,
    int64_t stride,
    int dim) const {
  return Stream(
      std::make_shared<stream::SlidingWindow>(self_, key, size, stride, dim));
}

Buffer Stream::to_buffer() {
  return Buffer(std::make_shared<buffer::FromStream>(self_));
}

Stream stream_csv_reader(
    const std::string& filename,
    char sep,
    char quote,
    const std::filesystem::path& local_prefix,
    std::shared_ptr<core::FileFetcher> fetcher) {
  return Stream(std::make_shared<stream::CSVReader>(
      filename, sep, quote, local_prefix, fetcher));
}

Stream stream_csv_reader(
    const std::shared_ptr<std::istream>& f,
    char sep,
    char quote,
    std::shared_ptr<core::FileFetcherHandle> file_handle) {
  return Stream(
      std::make_shared<stream::CSVReader>(f, sep, quote, file_handle));
}

Stream stream_csv_reader_from_string(
    const std::string& contents,
    char sep,
    char quote) {
  return Stream(std::make_shared<stream::CSVReader>(
      std::make_shared<std::istringstream>(contents), sep, quote));
}

Stream stream_line_reader(
    const std::string& filename,
    const std::string& key,
    bool unzip,
    const std::filesystem::path& local_prefix,
    std::shared_ptr<core::FileFetcher> fetcher) {
  return Stream(std::make_shared<stream::LineReader>(
      filename, key, unzip, local_prefix, fetcher));
}

Stream stream_line_reader(
    const std::shared_ptr<std::istream>& f,
    const std::string& key,
    bool unzip,
    std::shared_ptr<core::FileFetcherHandle> file_handle) {
  return Stream(
      std::make_shared<stream::LineReader>(f, key, unzip, file_handle));
}

} // namespace data
} // namespace mlx
