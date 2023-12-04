// Copyright © 2023 Apple Inc.

#include "mlx/data/stream/CSVReader.h"
#include "mlx/data/core/imemstream.h"

namespace mlx {
namespace data {
namespace stream {
CSVReader::CSVReader(
    const std::string& filename,
    char sep,
    char quote,
    const std::filesystem::path& local_prefix,
    std::shared_ptr<core::FileFetcher> fetcher) {
  if (fetcher) {
    fileHandle_ = fetcher->fetch(filename);
  }
  auto file_path = local_prefix / filename;
  csv_ = std::make_unique<core::CSVReader>(file_path.string(), sep, quote);
  keys_ = csv_->next();
}
CSVReader::CSVReader(
    const std::shared_ptr<std::istream>& f,
    char sep,
    char quote,
    std::shared_ptr<core::FileFetcherHandle> file_handle)
    : fileHandle_(file_handle) {
  csv_ = std::make_unique<core::CSVReader>(f, sep, quote);
  keys_ = csv_->next();
}
void CSVReader::reset() {
  std::unique_lock lock(mutex_);
  csv_->reset();
  csv_->next(); // keys
}

Sample CSVReader::next() const {
  std::vector<std::string> sample_str;
  {
    std::unique_lock lock(mutex_);
    sample_str = csv_->next();
  }
  if (sample_str.empty()) {
    return Sample();
  }
  if (sample_str.size() != keys_.size()) {
    throw std::runtime_error("CSVReader: inconsistent number of fields");
  }
  Sample sample;
  for (size_t i = 0; i < sample_str.size(); i++) {
    sample[keys_.at(i)] = std::make_shared<Array>(sample_str.at(i));
  }
  return sample;
}

CSVReaderFromKey::CSVReaderFromKey(
    std::shared_ptr<Stream> stream,
    const std::string& key,
    char sep,
    char quote,
    bool fromMemory,
    const std::filesystem::path& local_prefix,
    const std::shared_ptr<core::FileFetcher>& fetcher)
    : Compose(stream, [=](const Sample& sample) {
        if (fromMemory) {
          auto array =
              sample::check_key(sample, key, mlx::data::ArrayType::UInt8);
          auto ms = std::make_shared<core::imemstream>(array);
          return std::make_shared<CSVReader>(ms, sep, quote);
        } else {
          auto array =
              sample::check_key(sample, key, mlx::data::ArrayType::Int8);
          std::string filename(
              reinterpret_cast<char*>(array->data()), array->size());
          return std::make_shared<CSVReader>(
              filename, sep, quote, local_prefix, fetcher);
        }
      }) {}

} // namespace stream
} // namespace data
} // namespace mlx
