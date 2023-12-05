// Copyright © 2023 Apple Inc.

#pragma once

#include <filesystem>
#include <istream>
#include <mutex>

#include "bxzstr/bxzstr.hpp"

#include "mlx/data/core/FileFetcher.h"
#include "mlx/data/stream/Compose.h"
#include "mlx/data/stream/Stream.h"

namespace mlx {
namespace data {
namespace stream {

class LineReader : public Stream {
 public:
  LineReader(
      const std::string& filename,
      const std::string& key,
      bool unzip = false,
      const std::filesystem::path& local_prefix = "",
      std::shared_ptr<core::FileFetcher> fetcher = nullptr);
  LineReader(
      const std::shared_ptr<std::istream>& f,
      const std::string& key,
      bool unzip = false,
      std::shared_ptr<core::FileFetcherHandle> file_handle = nullptr);
  virtual Sample next() const override;
  void reset() override;

 private:
  void init_(const std::shared_ptr<std::istream>& f, bool unzip);
  Sample process_() const;
  std::string filename_;
  std::shared_ptr<std::istream> f_;
  std::shared_ptr<bxz::istream> uf_;
  std::string key_;
  std::shared_ptr<core::FileFetcherHandle> fileHandle_;
  mutable std::mutex mutex_;
};

class LineReaderFromKey : public Compose {
 public:
  LineReaderFromKey(
      std::shared_ptr<Stream> stream,
      const std::string& key,
      const std::string& dst_key,
      bool from_memory = false,
      bool unzip = false,
      const std::filesystem::path& local_prefix = "",
      std::shared_ptr<core::FileFetcher> fetcher = nullptr);
};

} // namespace stream
} // namespace data
} // namespace mlx
