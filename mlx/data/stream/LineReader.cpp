// Copyright © 2023 Apple Inc.

#include "mlx/data/stream/LineReader.h"
#include "mlx/data/core/imemstream.h"

#include <streambuf>

namespace mlx {
namespace data {
namespace stream {
LineReader::LineReader(
    const std::string& filename,
    const std::string& key,
    bool unzip,
    const std::filesystem::path& local_prefix,
    std::shared_ptr<core::FileFetcher> fetcher)
    : filename_(filename), key_(key) {
  if (fetcher) {
    fileHandle_ = fetcher->fetch(filename);
  }
  auto file_path = local_prefix / filename;
  init_(
      std::make_shared<std::ifstream>(
          file_path.string(), std::ios_base::binary),
      unzip);
}
void LineReader::init_(const std::shared_ptr<std::istream>& f, bool unzip) {
  f_ = f;
  if (unzip) {
    uf_ = std::make_shared<bxz::istream>(*f_);
  }
  if (!f_->good() || (uf_ && !uf_->good())) {
    throw std::runtime_error(
        "LineReader: could not open file <" + filename_ + ">");
  }
}
LineReader::LineReader(
    const std::shared_ptr<std::istream>& f,
    const std::string& key,
    bool unzip,
    std::shared_ptr<core::FileFetcherHandle> file_handle)
    : key_(key), fileHandle_(file_handle) {
  init_(f, unzip);
}
void LineReader::reset() {
  std::lock_guard<std::mutex> lock(mutex_);
  if (uf_) {
    uf_->clear();
    uf_->seekg(0);
  } else {
    f_->clear();
    f_->seekg(0);
  }
  if (!f_->good() || (uf_ && !uf_->good())) {
    throw std::runtime_error(
        "LineReader: could not seek to beginning of file <" + filename_ + ">");
  }
}

Sample LineReader::next() const {
  std::string line;
  {
    std::lock_guard<std::mutex> lock(mutex_);
    if (!std::getline((uf_ ? *uf_ : *f_), line)) {
      return Sample(); // EOF
    }
  }
  Sample sample;
  sample[key_] = std::make_shared<Array>(line);
  return sample;
}

LineReaderFromKey::LineReaderFromKey(
    std::shared_ptr<Stream> stream,
    const std::string& key,
    const std::string& dstKey,
    bool fromMemory,
    bool unzip,
    const std::filesystem::path& local_prefix,
    std::shared_ptr<core::FileFetcher> fetcher)
    : Compose(stream, [=](const Sample& sample) {
        if (fromMemory) {
          auto array =
              sample::check_key(sample, key, mlx::data::ArrayType::UInt8);
          auto ms = std::make_shared<core::imemstream>(array);
          return std::make_shared<LineReader>(ms, dstKey, unzip);
        } else {
          auto array =
              sample::check_key(sample, key, mlx::data::ArrayType::Int8);
          std::string filename(
              reinterpret_cast<char*>(array->data()), array->size());
          return std::make_shared<LineReader>(
              filename, dstKey, unzip, local_prefix, fetcher);
        }
      }) {}

} // namespace stream
} // namespace data
} // namespace mlx
